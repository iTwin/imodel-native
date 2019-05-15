/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/RadialGridPortion.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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

DEFINE_GRIDS_ELEMENT_BASE_METHODS (RadialGrid)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGrid::RadialGrid
(
CreateParams const& params
) : T_Super(params) 
    {
    if (params.m_classId.IsValid()) // elements created via handler have no classid.
        {
        SetDefaultAngleIncrement(params.m_defaultAngleIncrement);
        SetDefaultRadiusIncrement(params.m_defaultRadiusIncrement);
        SetDefaultStartAngle(params.m_defaultStartAngle);
        SetDefaultEndAngle(params.m_defaultEndAngle);
        SetDefaultStartRadius(params.m_defaultStartRadius);
        SetDefaultEndRadius(params.m_defaultEndRadius);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPtr        RadialGrid::Create
(
CreateParams const& params
)
    {
    return new RadialGrid (params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     10/17
//---------------------------------------------------------------------------------------
RadialGridPtr RadialGrid::CreateAndInsert (CreateParams const& params)
    {
    RadialGridPtr thisGrid = RadialGrid::Create(params);

    if (!thisGrid->Insert ().IsValid ())
        return nullptr;

    RadialAxisPtr planeAxis = RadialAxis::CreateAndInsert (*thisGrid);
    CircularAxisPtr arcAxis = CircularAxis::CreateAndInsert (*thisGrid);

    return thisGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPtr        RadialGrid::CreateAndInsertWithSurfaces
(
CreateParams const& params,
int radialSurfaceCount,
int circumferentialSurfaceCount
)
    {
    RadialGridPtr thisGrid = RadialGrid::CreateAndInsert(params);

    Dgn::SpatialLocationModelPtr subModel = thisGrid->GetSurfacesModel();

    PlanRadialGridSurface::CreateParams paramsRadial(*subModel, *thisGrid->GetRadialAxis(), 0.0, params.m_defaultStartRadius, params.m_defaultEndRadius, params.m_defaultStartElevation, params.m_defaultEndElevation);
    PlanCircumferentialGridSurface::CreateParams paramsCircular(*subModel, *thisGrid->GetCircularAxis(), params.m_defaultRadiusIncrement, params.m_defaultStartAngle, params.m_defaultEndAngle, params.m_defaultStartElevation, params.m_defaultEndElevation);

    for (int i = 0; i < radialSurfaceCount; i++)
        {
        PlanRadialGridSurface::CreateAndInsert(paramsRadial);
        paramsRadial.m_angle += params.m_defaultAngleIncrement;
        }

    for (int i = 0; i < circumferentialSurfaceCount; i++)
        {
        PlanCircumferentialGridSurface::CreateAndInsert(paramsCircular);
        paramsCircular.m_radius += params.m_defaultRadiusIncrement;
        }

    return thisGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RadialAxisCPtr        RadialGrid::GetRadialAxis
(
) const
    {
    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(GRIDS_SCHEMA(GRIDS_CLASS_RadialAxis), "WHERE Model.Id=?", "ORDER BY ECInstanceId ASC");
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement())
        {
        pStmnt->BindId(1, GetSubModelId());
        }

    if (iterator == iterator.end())
        return nullptr;

    return GetDgnDb().Elements().Get<RadialAxis>((*iterator.begin()).GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CircularAxisCPtr        RadialGrid::GetCircularAxis
(
) const
    {
    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(GRIDS_SCHEMA(GRIDS_CLASS_CircularAxis), "WHERE Model.Id=?", "ORDER BY ECInstanceId ASC");
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement())
        {
        pStmnt->BindId(1, GetSubModelId());
        }

    if (iterator == iterator.end())
        return nullptr;

    return GetDgnDb().Elements().Get<CircularAxis>((*iterator.begin()).GetElementId());
    }

END_GRIDS_NAMESPACE