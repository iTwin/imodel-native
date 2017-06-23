/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/OrthogonalGridPortion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/OrthogonalGridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include "PublicApi\GridPlaneSurface.h"
#include <DimensionHandler.h>
#include <BuildingUtils.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (OrthogonalGridPortion)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPortion::OrthogonalGridPortion
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPortionPtr        OrthogonalGridPortion::Create
(
Dgn::DgnModelCR model
)
    {
    return new OrthogonalGridPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
DVec3d OrthogonalGridPortion::FindOrthogonalFormTranslation(int elementIndex, double interval, double rotationAngle, bool isHorizontal)
    {
    double offset;
    if (isHorizontal)
        offset = elementIndex * interval;
    else
        offset = -elementIndex * interval;

    DPoint3d startingPoint = DPoint3d::FromZero();
    startingPoint.y += offset;
    DVec3d translation = DVec3d::FromStartEnd(DPoint3d::FromZero(), startingPoint);
    translation.RotateXY(rotationAngle);
    return translation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
void OrthogonalGridPortion::AddDimensionsToOrthogonalGrid
(
    GridSurfacePtr lastElement,
    GridSurfacePtr element,
    double distance
)
    {
    DPlane3d elementPlane = (dynamic_cast<GridPlaneSurface *>(element.get()))->GetPlane();
    DVec3d planeNormal = elementPlane.normal;

    if (lastElement.IsValid())
        {
        DPoint3d elementOrigin = elementPlane.origin;
        DPoint3d lastPlaneOrigin;
        DPlane3d lastPlane = (dynamic_cast<GridPlaneSurface *>(lastElement.get()))->GetPlane();
        bsiDPlane3d_projectPoint(&lastPlane, &lastPlaneOrigin, &elementOrigin);

        DVec3d direction = DVec3d::FromStartEnd(lastPlaneOrigin, elementOrigin);
        bsiDVec3d_normalizeInPlace(&direction);

        DimensionHandler::Insert(lastElement->GetDgnDb(),
                                 lastElement->GetElementId(),
                                 element->GetElementId(),
                                 0, 0,
                                 direction,
                                 distance);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridElementVector OrthogonalGridPortion::CreateGridElements(CreateParams params, bool isHorizontal)
    {
    GridElementVector orthogonalGrid;

    DgnExtrusionDetail extDetail = CreatePlaneGridExtrusionDetail(params.m_length, params.m_height);
    GridPlaneSurfacePtr baseGridPlane = dynamic_cast<GridPlaneSurface *>(CreateGridSurface(*params.m_model, extDetail, GridType::GRID_TYPE_Orthogonal).get());
    if (!baseGridPlane.IsValid())
        return GridElementVector();

    baseGridPlane->MoveToPoint(params.m_origin);
    baseGridPlane->RotateXY(params.m_rotationAngle);

    if (isHorizontal)
        for (int i = 0; i < params.m_horizontalCount; ++i)
            {
            GridPlaneSurfacePtr orthogonalGridPlane = dynamic_cast<GridPlaneSurface *>(baseGridPlane->Clone().get());
            if (!orthogonalGridPlane.IsValid())
                return GridElementVector();

            orthogonalGridPlane->TranslateXY(params.m_horizontalExtendTranslation);
            orthogonalGridPlane->TranslateXY(FindOrthogonalFormTranslation(i, params.m_horizontalInterval, params.m_rotationAngle, true));
            orthogonalGrid.push_back(orthogonalGridPlane);
            }
    else
        for (int i = 0; i < params.m_verticalCount; ++i)
            {
            GridPlaneSurfacePtr orthogonalGridPlane = dynamic_cast<GridPlaneSurface *>(baseGridPlane->Clone().get());
            if (!orthogonalGridPlane.IsValid())
                return GridElementVector();

            orthogonalGridPlane->RotateXY(msGeomConst_pi / 2);
            orthogonalGridPlane->TranslateXY(params.m_verticalExtendTranslation);
            orthogonalGridPlane->TranslateXY(FindOrthogonalFormTranslation(i, params.m_verticalInterval, params.m_rotationAngle + msGeomConst_pi / 2, false));
            orthogonalGrid.push_back(orthogonalGridPlane);
            }

    return orthogonalGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridElementVector OrthogonalGridPortion::CreateGridPreview(CreateParams params)
    {
    GridElementVector horizontalElements = CreateGridElements(params, true);
    GridElementVector verticalElements = CreateGridElements(params, false);

    GridElementVector orthogonalGrid = horizontalElements;
    orthogonalGrid.insert(orthogonalGrid.end(), verticalElements.begin(), verticalElements.end());

    return orthogonalGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus OrthogonalGridPortion::CreateAndInsert(CreateParams params)
    {
    GridAxisMap grid;
    GridElementVector horizontalElements = CreateGridElements(params, true);
    GridElementVector verticalElements = CreateGridElements(params, false);

    grid[HORIZONTAL_AXIS] = horizontalElements;
    grid[VERTICAL_AXIS] = verticalElements;

    for (bpair<Utf8String, GridElementVector> pair : grid)
        {
        GridSurfacePtr lastElement;
        double distance = (0 == strcmp(pair.first.c_str(), HORIZONTAL_AXIS) ? params.m_horizontalInterval : params.m_verticalInterval);

        for (GridSurfacePtr gridSurface : pair.second)
            {
            if (BentleyStatus::ERROR == BuildingUtils::InsertElement(gridSurface))
                return BentleyStatus::ERROR;

            AddDimensionsToOrthogonalGrid(lastElement, gridSurface, distance);
            lastElement = gridSurface;
            }
        }

    return BentleyStatus::SUCCESS;
    }

END_GRIDS_NAMESPACE