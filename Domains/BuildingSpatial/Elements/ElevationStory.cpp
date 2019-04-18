/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ElevationStory.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/ElevationStory.h"

BEGIN_BUILDINGSPATIAL_NAMESPACE

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

END_BUILDINGSPATIAL_NAMESPACE
