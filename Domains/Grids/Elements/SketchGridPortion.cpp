#include "PublicApi/SketchGridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <ConstraintSystem/ConstraintSystemApi.h>


BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (SketchGridPortion)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGridPortion::SketchGridPortion
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGridPortion::SketchGridPortion
(
T_Super::CreateParams const& params,
DVec3d                      normal
) : T_Super(params, normal) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGridPortionPtr        SketchGridPortion::Create
(
Dgn::DgnModelCR model,
DVec3d          normal,
Utf8CP          name
)
    {
    return new SketchGridPortion(GeometricElement3d::CreateParams(model.GetDgnDb(),
                                                                  model.GetModelId(),
                                                                  QueryClassId(model.GetDgnDb()),
                                                                  SpatialCategory::QueryCategoryId(model.GetDgnDb().GetDictionaryModel(), GRIDS_CATEGORY_CODE_Uncategorized),
                                                                  Placement3d(),
                                                                  Dgn::DgnCode(model.GetDgnDb().CodeSpecs().QueryCodeSpecId(GRIDS_AUTHORITY_SketchGridPortion),
                                                                               model.GetModeledElementId(),
                                                                               name)),
                                 normal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
SketchGridPortionPtr SketchGridPortion::TryGet(Dgn::DgnDbR db, Dgn::DgnElementId parentId, Utf8CP gridName)
    {
    return db.Elements().GetForEdit<Grids::SketchGridPortion>(BuildingElementsUtils::GetElementIdByParentElementAuthorityAndName(db,
                                                                                                                           GRIDS_AUTHORITY_SketchGridPortion,
                                                                                                                           parentId,
                                                                                                                           gridName));
    }


END_GRIDS_NAMESPACE