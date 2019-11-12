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
#include <BeSQLite/BeSQLite.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS(OrthogonalGrid)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGrid::OrthogonalGrid
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid()) // elements created via handler have no classid.
        {
        SetDefaultCoordinateIncrementX(params.m_defaultCoordinateIncrementX);
        SetDefaultCoordinateIncrementY(params.m_defaultCoordinateIncrementY);
        SetDefaultStartExtentX(params.m_defaultStartExtentX);
        SetDefaultEndExtentX(params.m_defaultEndExtentX);
        SetDefaultStartExtentY(params.m_defaultStartExtentY);
        SetDefaultEndExtentY(params.m_defaultEndExtentY);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPtr        OrthogonalGrid::Create
(
CreateParams const& params
)
    {
    OrthogonalGridPtr thisGrid = new OrthogonalGrid (params);
    
    return thisGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPtr        OrthogonalGrid::CreateAndInsert
(
CreateParams const& params
)
    {
    OrthogonalGridPtr thisGrid = OrthogonalGrid::Create(params);

    if (!thisGrid->Insert().IsValid())
        return nullptr;

    OrthogonalAxisXPtr horizontalAxis = OrthogonalAxisX::CreateAndInsert(*thisGrid);
    OrthogonalAxisYPtr verticalAxis = OrthogonalAxisY::CreateAndInsert(*thisGrid);

    return thisGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPtr        OrthogonalGrid::CreateAndInsertWithSurfaces
(
CreateParams const& params,
int xSurfaceCount,
int ySurfaceCount
)
    {
    OrthogonalGridPtr thisGrid = OrthogonalGrid::CreateAndInsert(params);
    if (thisGrid.IsNull())
        return nullptr;

    Dgn::SpatialLocationModelPtr subModel = thisGrid->GetSurfacesModel();

    PlanCartesianGridSurface::CreateParams paramsX(*subModel, *thisGrid->GetXAxis(), 0.0, params.m_defaultStartExtentX, params.m_defaultEndExtentX, params.m_defaultStartElevation, params.m_defaultEndElevation);
    PlanCartesianGridSurface::CreateParams paramsY(*subModel, *thisGrid->GetYAxis(), 0.0, params.m_defaultStartExtentY, params.m_defaultEndExtentY, params.m_defaultStartElevation, params.m_defaultEndElevation);

    for (int i = 0; i < xSurfaceCount; i++)
        {
        PlanCartesianGridSurface::CreateAndInsert(paramsX);
        paramsX.m_coordinate += params.m_defaultCoordinateIncrementX;
        }

    for (int i = 0; i < ySurfaceCount; i++)
        {
        PlanCartesianGridSurface::CreateAndInsert(paramsY);
        paramsY.m_coordinate += params.m_defaultCoordinateIncrementY;
        }

    return thisGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisXCPtr        OrthogonalGrid::GetXAxis
(
) const
    {
    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(GRIDS_SCHEMA(GRIDS_CLASS_OrthogonalAxisX), "WHERE Model.Id=?", "ORDER BY ECInstanceId ASC");
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement())
        {
        pStmnt->BindId(1, GetSubModelId());
        }

    if (iterator == iterator.end())
        return nullptr;

    return GetDgnDb().Elements().Get<OrthogonalAxisX>((*iterator.begin()).GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisYCPtr        OrthogonalGrid::GetYAxis
(
) const
    {
    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(GRIDS_SCHEMA(GRIDS_CLASS_OrthogonalAxisY), "WHERE Model.Id=?", "ORDER BY ECInstanceId ASC");
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement())
        {
        pStmnt->BindId(1, GetSubModelId());
        }

    if (iterator == iterator.end())
        return nullptr;

    return GetDgnDb().Elements().Get<OrthogonalAxisY>((*iterator.begin()).GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
DVec3d OrthogonalGrid::FindOrthogonalFormTranslation(int elementIndex, double interval, double rotationAngle, bool isHorizontal)
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

END_GRIDS_NAMESPACE