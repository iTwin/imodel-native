/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Grids/Elements/GridElementsAPI.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>


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
SketchGridPtr SketchGrid::Create
(
Dgn::SpatialModelCR model,
Dgn::DgnElementId scopeElementId,
Utf8CP name,
double defaultStartElevation,
double defaultEndElevation
)
    {
    return new SketchGrid(PlanGrid::CreateParams(model, scopeElementId, name, QueryClassId(model.GetDgnDb()), defaultStartElevation, defaultEndElevation));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGridPtr SketchGrid::CreateAndInsert
(
Dgn::SpatialModelCR model,
Dgn::DgnElementId scopeElementId,
Utf8CP name,
double defaultStartElevation,
double defaultEndElevation
)
    {
    SketchGridPtr grid = SketchGrid::Create(model, scopeElementId, name, defaultStartElevation, defaultEndElevation);

    if(grid.IsNull())
        return nullptr;

    if(grid->Insert().IsNull())
        return nullptr;

    return grid;
    }

END_GRIDS_NAMESPACE