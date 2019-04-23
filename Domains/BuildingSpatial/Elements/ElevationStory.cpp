/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ElevationStory.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/ElevationStory.h"
#include "BuildingShared/BuildingSharedApi.h"

USING_NAMESPACE_BUILDING_SHARED

BEGIN_BUILDINGSPATIAL_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::Render::GeometryParams ElevationStory::_CreateGeometryParameters()
    {
    Dgn::Render::GeometryParams parameters(Dgn::SpatialCategory::QueryCategoryId(GetDgnDb().GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_ElevationStory));
    parameters.SetFillTransparency(0.5);
    parameters.SetLineColor(Dgn::ColorDef::White());
    return parameters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nerijus.Jakeliunas               10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnSubCategoryId ElevationStory::_GetLabelSubCategoryId() const
    {
    auto categoryId = Dgn::SpatialCategory::QueryCategoryId(GetDgnDb().GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_ElevationStory);
    return Dgn::DgnSubCategory::QuerySubCategoryId(GetDgnDb(), Dgn::DgnSubCategory::CreateCode(GetDgnDb(), categoryId, BUILDINGSPATIAL_SUBCATEGORY_CODE_SpatialElementLabels));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnClassId ElevationStory::QueryClassId (Dgn::DgnDbR db)
    {
    return Dgn::DgnClassId(db.Schemas().GetClassId(BUILDINGSPATIAL_SCHEMA_NAME, BUILDINGSPATIAL_CLASS_ElevationStory));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationStoryPtr ElevationStory::Create (Dgn::DgnModelCR model)
    {
    Dgn::DictionaryModelR dictionaryModel = model.GetDgnDb().GetDictionaryModel();
    Dgn::DgnCategoryId categoryId = Dgn::SpatialCategory::QueryCategoryId (dictionaryModel, BUILDINGSPATIAL_CATEGORY_CODE_ElevationStory);

    if (!categoryId.IsValid())
        return nullptr;

    Dgn::DgnClassId classId = QueryClassId(model.GetDgnDb());
    Dgn::GeometricElement3d::CreateParams createParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId);
    
    return new ElevationStory(createParams);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
bool ElevationStory::SetFootprintShape
(
    CurveVectorCPtr curveVector,
    bool updatePlacementOrigin
)
    {
    Dgn::Render::GeometryParams geometryParameters = _CreateGeometryParameters();
    return DgnGeometryUtils::UpdateExtrusionGeometry(*this, &geometryParameters, _GetLabelSubCategoryId(), curveVector, updatePlacementOrigin);
    }

END_BUILDINGSPATIAL_NAMESPACE
