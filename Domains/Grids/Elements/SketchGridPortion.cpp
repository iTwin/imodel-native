
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <ConstraintSystem/ConstraintSystemApi.h>


BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (SketchGrid)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGrid::SketchGrid
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGridPtr        SketchGrid::Create
(
Dgn::DgnModelCR model,
Utf8CP          name
)
    {
    return new SketchGrid(GeometricElement3d::CreateParams(model.GetDgnDb(),
                                                                  model.GetModelId(),
                                                                  QueryClassId(model.GetDgnDb()),
                                                                  DgnCategoryId(),
                                                                  Placement3d(),
                                                                  Dgn::DgnCode(model.GetDgnDb().CodeSpecs().QueryCodeSpecId(GRIDS_AUTHORITY_Grid),
                                                                               model.GetModeledElementId(),
                                                                               name)));
    }


END_GRIDS_NAMESPACE