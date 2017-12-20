\
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <BuildingShared/BuildingSharedApi.h>


BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

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
Dgn::SpatialLocationModelCR model,
Utf8CP          name
)
    {
    return new SketchGrid(Grid::CreateParams(&model, model.GetModeledElementId (), name, QueryClassId(model.GetDgnDb())));
    }


END_GRIDS_NAMESPACE