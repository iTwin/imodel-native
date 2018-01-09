/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/ArcGridPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
BentleyStatus CSEArcGridPlacementStrategy::GetArcPoints(DPoint3dR center, DPoint3dR start, DPoint3dR end)
    {
    if (m_points.size() < 2)
        return BentleyStatus::ERROR;

    center = m_points[0];
    start = m_points[1];

    if (m_points.size() > 2)
        end = m_points[2];
    else if (m_useDynamicPoint)
        end = m_dynamicPoint;
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr CSEArcGridPlacementStrategy::_CreateAndInsertGridSurface()
    {
    if (GetGrid().IsNull() || GetAxis().IsNull())
        return nullptr;

    DPoint3d center, start, end;
    if (BentleyStatus::ERROR == GetArcPoints(center, start, end))
        return nullptr;

    SketchArcGridSurface::CreateParams params(*GetGrid()->GetSurfacesModel(), *GetAxis(), center.z, center.z + GetHeight(), GeometryUtils::CreateDEllipse3dArc(center, start, end, m_ccw));
    GridArcSurfacePtr surface = SketchArcGridSurface::Create(params);
    if (surface->Insert().IsValid())
        return surface;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
BentleyStatus CSEArcGridPlacementStrategy::_UpdateGridSurface()
    {
    if (m_surface.IsNull())
        return BentleyStatus::SUCCESS; // Nothing to update

                                       // Correct grid's axis
    if (GetAxis()->GetElementId() != m_surface->GetAxisId())
        m_surface->SetAxisId(GetAxis()->GetElementId());

    m_surface->SetStartElevation(GetElevation());
    // Correct grid's height
    double actualHeight;
    if (BentleyStatus::SUCCESS != m_surface->TryGetHeight(actualHeight))
        return BentleyStatus::ERROR;

    if (actualHeight != GetHeight())
        {
        m_surface->SetEndElevation(GetHeight() + m_surface->GetStartElevation());
        }

    if (m_surface->Update().IsValid())
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
BentleyStatus CSEArcGridPlacementStrategy::_UpdateByKeyPoints()
    {
    if (m_surface.IsNull())
        return BentleyStatus::SUCCESS; // Nothing to update

    DPoint3d center, start, end;
    if (BentleyStatus::ERROR == GetArcPoints(center, start, end))
        return BentleyStatus::ERROR;

    m_surface->SetBaseArc(GeometryUtils::CreateDEllipse3dArc(center, start, end, m_ccw));

    if (m_surface->Update().IsValid())
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr CSEArcGridPlacementStrategy::_GetGridSurface()
    {
    return m_surface;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
void CSEArcGridPlacementStrategy::_SetGridSurface(Grids::GridSurfacePtr surface)
    {
    m_surface = dynamic_cast<SketchArcGridSurface*>(surface.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
size_t CSEArcGridPlacementStrategy::_GetMaxPointCount()
    {
    return 3;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
void CSEArcGridPlacementStrategy::_SetDynamicPoint(DPoint3d point)
    {
   
    if (2 == m_points.size())
        {
        // Recalculate ccw if needed
        if (!m_useDynamicPoint || fabs(DVec3d::FromStartEnd(m_points[0], m_points[1]).AngleToXY(
                                       DVec3d::FromStartEnd(m_points[0], m_dynamicPoint))) < 
                                  fabs(DVec3d::FromStartEnd(m_points[0], m_points[1]).AngleToXY(
                                       DVec3d::FromStartEnd(m_points[0], point))))
            m_ccw = DVec3d::FromStartEnd(m_points[0], m_points[1]).AngleToXY(
                DVec3d::FromStartEnd(m_points[0], point)) > 0; // Choose direction with smaller angle
        
        // adjust point to be on the arc
        double toCenter = m_points[0].DistanceXY(point);
        double radius = m_points[0].DistanceXY(m_points[1]);
        point.Scale(radius / toCenter);
        }

    T_Super::_SetDynamicPoint(point);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
CSEArcGridPlacementStrategyPtr CSEArcGridPlacementStrategy::Create(Dgn::DgnDbR db, double elevation, double height, Grids::SketchGridPtr grid)
    {
    return new CSEArcGridPlacementStrategy(db, elevation, height, grid);
    }

END_GRIDS_NAMESPACE

