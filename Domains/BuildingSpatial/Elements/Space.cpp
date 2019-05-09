/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/Space.h"
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDING_SHARED

BEGIN_BUILDINGSPATIAL_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::Render::GeometryParams Space::_CreateGeometryParameters()
    {
    Dgn::Render::GeometryParams parameters(Dgn::SpatialCategory::QueryCategoryId(GetDgnDb().GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_Space));
    parameters.SetFillTransparency(0.5);
    parameters.SetLineColor(Dgn::ColorDef::White());
    return parameters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nerijus.Jakeliunas               10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnSubCategoryId Space::_GetLabelSubCategoryId() const
    {
    auto categoryId = Dgn::SpatialCategory::QueryCategoryId(GetDgnDb().GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_Space);
    return Dgn::DgnSubCategory::QuerySubCategoryId(GetDgnDb(), Dgn::DgnSubCategory::CreateCode(GetDgnDb(), categoryId, BUILDINGSPATIAL_SUBCATEGORY_CODE_SpatialElementLabels));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnClassId Space::QueryClassId(Dgn::DgnDbR db)
    {
    return Dgn::DgnClassId(db.Schemas().GetClassId(BUILDINGSPATIAL_SCHEMA_NAME, BUILDINGSPATIAL_CLASS_Space));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
SpacePtr Space::Create (Dgn::DgnModelCR model)
    {
    Dgn::DictionaryModelR dictionaryModel = model.GetDgnDb ().GetDictionaryModel ();
    Dgn::DgnCategoryId categoryId = Dgn::SpatialCategory::QueryCategoryId (dictionaryModel, BUILDINGSPATIAL_CATEGORY_CODE_Space);

    if (!categoryId.IsValid ())
        return nullptr;

    Dgn::DgnClassId classId = QueryClassId (model.GetDgnDb ());
    Dgn::GeometricElement3d::CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);
    
    return new Space (createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool Space::SetFootprintShape
(
    CurveVectorCPtr curveVector,
    bool updatePlacementOrigin
)
    {
    Dgn::Render::GeometryParams geometryParameters = _CreateGeometryParameters();

    return DgnGeometryUtils::UpdateExtrusionGeometry(*this, &geometryParameters, _GetLabelSubCategoryId(), curveVector, updatePlacementOrigin);
    }
    
END_BUILDINGSPATIAL_NAMESPACE
