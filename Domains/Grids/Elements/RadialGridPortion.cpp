/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/RadialGridPortion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/RadialGridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include "PublicApi\GridPlaneSurface.h"
#include "PublicApi\GridArcSurface.h"
#include <GeometryUtils.h>
#include <UnitConverter.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (RadialGridPortion)

#define CIRCULAR_GRID_EXTEND_LENGTH 5

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPortion::RadialGridPortion
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPortionPtr        RadialGridPortion::Create
(
Dgn::SpatialLocationModelCR model
)
    {
    return new RadialGridPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridElementVector RadialGridPortion::CreateGridPreview(CreateParams params)
    {
    GridElementVector radialGrid;

    DgnExtrusionDetail extDetail = GeometryUtils::CreatePlaneExtrusionDetail(params.m_length, params.m_height);
    GridPlaneSurfacePtr baseGridPlane = GridPlaneSurface::Create(*params.m_model, extDetail);
    if (!baseGridPlane.IsValid())
        return GridElementVector();

    // Create plane grids
    for (int i = 0; i <= params.m_planeCount; ++i)
        {
        GridPlaneSurfacePtr planeSurface = dynamic_cast<GridPlaneSurface *>(baseGridPlane->Clone().get());
        if (!planeSurface.IsValid())
            return GridElementVector();

        planeSurface->RotateXY(i * params.m_planeIterationAngle);
        radialGrid.push_back(planeSurface);
        }

    for (int i = 0; i < params.m_circularCount; ++i)
        {
        DgnExtrusionDetail extDetail = GeometryUtils::CreateArcExtrusionDetail((i + 1) * params.m_circularInterval, params.m_planeIterationAngle * params.m_planeCount, params.m_height, UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH));
        GridArcSurfacePtr arcSurface = GridArcSurface::Create(*params.m_model, extDetail);
        if (!arcSurface.IsValid())
            return GridElementVector();

        radialGrid.push_back(arcSurface);
        }

    return radialGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus RadialGridPortion::CreateAndInsert(CreateParams params)
    {
    GridElementVector radialGrid = CreateGridPreview(params);
    GridAxisMap grid;
    grid[DEFAULT_AXIS] = radialGrid;

    return InsertGridMapElements(grid);
    }

END_GRIDS_NAMESPACE