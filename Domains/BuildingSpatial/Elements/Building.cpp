/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <BuildingSpatial/Elements/Building.h>

BEGIN_BUILDINGSPATIAL_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/

Dgn::DgnClassId Building::QueryClassId(Dgn::DgnDbR db)
    {
    return Dgn::DgnClassId(db.Schemas().GetClassId(BUILDINGSPATIAL_SCHEMA_NAME, BUILDINGSPATIAL_CLASS_Building));
    }

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

END_BUILDINGSPATIAL_NAMESPACE
