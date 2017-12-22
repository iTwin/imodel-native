/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/LineGridPlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "Grids\GridsApi.h"
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_BENTLEY_DGN

BEGIN_GRIDS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
BentleyStatus LineGridPlacementStrategy::GetEndPoints(DPoint3dR start, DPoint3dR end)
    {
    if (m_points.empty())
        {
        if (m_useDynamicPoint)
            start = end = m_dynamicPoint;
        else
            return BentleyStatus::ERROR;
        }
    else if (1 == m_points.size())
        {
        start = m_points[0];
        if (m_useDynamicPoint)
            end = m_dynamicPoint;
        else
            end = start;
        }
    else
        {
        start = m_points[0];
        end = m_points[1];
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr LineGridPlacementStrategy::_CreateAndInsertGridSurface()
    {
    if (GetGrid().IsNull() || GetAxis().IsNull())
        return nullptr;

    DPoint3d startPoint, endPoint;
    if (BentleyStatus::ERROR == GetEndPoints(startPoint, endPoint))
        return nullptr;
    
    DgnExtrusionDetail shape = GeometryUtils::CreatePlaneExtrusionDetail(startPoint, endPoint, GetHeight());
    GridPlanarSurfacePtr surface = GridPlanarSurface::Create(*GetGrid()->GetSurfacesModel(), *GetAxis(), shape);
    if (surface->Insert().IsValid())
        return surface;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
BentleyStatus LineGridPlacementStrategy::_UpdateGridSurface()
    {
    if (m_surface.IsNull())
        return BentleyStatus::SUCCESS; // Nothing to update

    // Correct grid's axis
    if (GetAxis()->GetElementId() != m_surface->GetAxisId())
        m_surface->SetAxisId(GetAxis()->GetElementId());

    // Correct grid's height
    double actualHeight;
    if (BentleyStatus::SUCCESS != m_surface->TryGetHeight(actualHeight))
        return BentleyStatus::ERROR;
    
    if (actualHeight != GetHeight())
        {
        if (BentleyStatus::SUCCESS != m_surface->SetHeight(GetHeight()))
            return BentleyStatus::ERROR;
        }

    // Correct grid's elevation
    if (m_surface->GetPlacement().GetOrigin().z != GetElevation())
        m_surface->SetElevation(GetElevation());

    if (m_surface->Update().IsValid())
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
BentleyStatus LineGridPlacementStrategy::_UpdateByKeyPoints()
    {
    if (m_surface.IsNull())
        return BentleyStatus::SUCCESS; // Nothing to update

    DPoint3d startPoint, endPoint;
    if (BentleyStatus::ERROR == GetEndPoints(startPoint, endPoint))
        return BentleyStatus::ERROR;

    CurveVectorPtr newBase = CurveVector::CreateLinear({ startPoint, endPoint }, CurveVector::BoundaryType::BOUNDARY_TYPE_None);
    if (newBase.IsNull())
        return BentleyStatus::ERROR;

    if (BentleyStatus::ERROR == m_surface->SetBaseCurve(newBase))
        return BentleyStatus::ERROR;

    if (m_surface->Update().IsValid())
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr LineGridPlacementStrategy::_GetGridSurface()
    {
    return m_surface;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
void LineGridPlacementStrategy::_SetGridSurface(Grids::GridSurfacePtr surface)
    {
    m_surface = dynamic_cast<GridPlanarSurface*>(surface.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
size_t LineGridPlacementStrategy::_GetMaxPointCount()
    {
    return 2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
LineGridPlacementStrategyPtr LineGridPlacementStrategy::Create(Dgn::DgnDbR db, double elevation, double height, Grids::SketchGridPtr grid)
    {
    LineGridPlacementStrategyPtr strategy = new LineGridPlacementStrategy(db, elevation, height, grid);
    return strategy;
    }

END_GRIDS_NAMESPACE

