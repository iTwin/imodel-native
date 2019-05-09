/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/Building.h"
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDING_SHARED

BEGIN_BUILDINGSPATIAL_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::Render::GeometryParams Building::_CreateGeometryParameters()
    {
    Dgn::Render::GeometryParams parameters(Dgn::SpatialCategory::QueryCategoryId(GetModel()->GetDgnDb().GetDictionaryModel(), BUILDINGSPATIAL_CATEGORY_CODE_Building));

    return parameters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/

Dgn::DgnClassId Building::QueryClassId(Dgn::DgnDbR db)
    {
    return Dgn::DgnClassId(db.Schemas().GetClassId(BUILDINGSPATIAL_SCHEMA_NAME, BUILDINGSPATIAL_CLASS_Building));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BuildingPtr Building::Create (Dgn::DgnModelCR model)
    { 
    Dgn::DictionaryModelR dictionaryModel = model.GetDgnDb ().GetDictionaryModel ();
    Dgn::DgnCategoryId categoryId = Dgn::SpatialCategory::QueryCategoryId (dictionaryModel, BUILDINGSPATIAL_CATEGORY_CODE_Building);

    if (!categoryId.IsValid ())
        return nullptr;

    Dgn::DgnClassId classId = QueryClassId(model.GetDgnDb());
    Dgn::GeometricElement3d::CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);
    
    return new Building (createParams);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
bool Building::SetFootprintShape
(
    CurveVectorCPtr curveVector,
    bool updatePlacementOrigin
)
    {
    Dgn::Render::GeometryParams geometryParameters = _CreateGeometryParameters();

    return DgnGeometryUtils::UpdateExtrusionGeometry(*this, &geometryParameters, Dgn::DgnSubCategoryId(), curveVector, updatePlacementOrigin);
    }

END_BUILDINGSPATIAL_NAMESPACE
