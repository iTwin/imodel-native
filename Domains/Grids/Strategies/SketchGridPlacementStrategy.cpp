/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/SketchGridPlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "Grids\GridsApi.h"

USING_NAMESPACE_BENTLEY_DGN

BEGIN_GRIDS_NAMESPACE 

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridAxisPtr SketchGridPlacementStrategy::_CreateAndInsertNewAxis()
    {
    Dgn::DefinitionModelCR defModel = GetDb().GetDictionaryModel();
    return GridAxis::CreateAndInsert(defModel, *m_grid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
GridSurfacePtr SketchGridPlacementStrategy::GetGridSurface()
    {
    GridSurfacePtr surface;
    if (IsInDynamics())
        {
        surface = _GetGridSurface();
        if (surface.IsValid())
            return surface;
        }
   
    surface = _CreateAndInsertGridSurface();
    
    if (surface.IsValid())
        {
        _SetGridSurface(surface);
        _BeginDynamics();
        }
    
    return surface;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr SketchGridPlacementStrategy::Finish()
    {
    if (m_points.empty())
        return _GetGridSurface(); // Can't create a grid surface with less than 2 points

    m_dynamicPoint = m_points.back();

    if (BentleyStatus::ERROR == _UpdateGridSurface() ||
        BentleyStatus::ERROR == _UpdateByKeyPoints())
        return _GetGridSurface();
    
    m_points = {};
    m_dynamicPoint = DPoint3d();
    _EndDynamics();

    return _GetGridSurface();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr SketchGridPlacementStrategy::SetBottomElevation(double elevation)
    {
    if (elevation > m_height + m_elevation)
        return _GetGridSurface();

    m_height -= elevation - m_elevation;
    BeAssert(m_height > 0 && "Grid height should not be negative");
    
    m_elevation = elevation;

    for (DPoint3d & point : m_points)
        point.z = elevation;

    m_dynamicPoint.z = elevation;
    
    if (BentleyStatus::ERROR == _UpdateGridSurface())
        return nullptr;
    
    return GetGridSurface();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr SketchGridPlacementStrategy::SetTopElevation(double elevation)
    {
    if (elevation < m_elevation)
        return _GetGridSurface();

    m_height = elevation - m_elevation;
    BeAssert(m_height > 0 && "Grid height should not be negative");

    if (BentleyStatus::ERROR == _UpdateGridSurface())
        return nullptr;
    
    return GetGridSurface();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr SketchGridPlacementStrategy::SetAxis(Grids::GridAxisPtr axis)
    {
    if (axis.IsNull() || axis->GetGridId() != m_grid->GetElementId())
        return _GetGridSurface();

    m_axis = axis;

    if (BentleyStatus::ERROR == _UpdateGridSurface())
        return nullptr;
    
    return GetGridSurface();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr SketchGridPlacementStrategy::SetGridAndAxis(Grids::SketchGridPtr grid, Grids::GridAxisPtr axis)
    {
    if (grid.IsNull())
        return _GetGridSurface();

    m_grid = grid;

    if (axis.IsNull())
        m_axis = _CreateAndInsertNewAxis();
    else
        m_axis = axis;

    if (BentleyStatus::ERROR == _UpdateGridSurface())
        return nullptr;

    return GetGridSurface();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr SketchGridPlacementStrategy::AcceptDynamicPoint()
    {
    if (m_points.size() == _GetMaxPointCount())
        return _GetGridSurface();

    _AcceptDynamicPoint();
    m_useDynamicPoint = false;
    
    if (BentleyStatus::ERROR == _UpdateByKeyPoints())
        return nullptr;

    return GetGridSurface();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              12/2017
//---------------------------------------------------------------------------------------
Grids::GridSurfacePtr SketchGridPlacementStrategy::SetDynamicPoint(DPoint3d point)
    {
    DPoint3d adjusted = point;
    adjusted.z = m_elevation;

    _SetDynamicPoint(adjusted);
    m_useDynamicPoint = true;

    if (BentleyStatus::ERROR == _UpdateByKeyPoints())
        return nullptr;

    return GetGridSurface();
    }

END_GRIDS_NAMESPACE