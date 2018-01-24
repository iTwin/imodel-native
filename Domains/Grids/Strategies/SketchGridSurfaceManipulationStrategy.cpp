/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/SketchGridSurfaceManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids\GridsApi.h>

USING_NAMESPACE_GRIDS
USING_NAMESPACE_BUILDING_SHARED

const Utf8CP SketchGridSurfaceManipulationStrategy::prop_BottomElevation = "BottomElevation";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_TopElevation = "TopElevation";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_Axis = "Axis";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_Name = "Name";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_Grid = "Grid";

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
ISolidPrimitivePtr SketchGridSurfaceManipulationStrategy::FinishGeometry() const
    {
    return m_geometryManipulationStrategy->FinishSolidPrimitive();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
SketchGridSurfaceManipulationStrategy::SketchGridSurfaceManipulationStrategy
(
    BBS::ExtrusionManipulationStrategyR geometryManipulationStrategy
)
    : T_Super()
    , m_geometryManipulationStrategy(&geometryManipulationStrategy)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> SketchGridSurfaceManipulationStrategy::_GetKeyPoints() const
    {
    return m_geometryManipulationStrategy->GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    return m_geometryManipulationStrategy->IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_ResetDynamicKeyPoint()
    {
    return m_geometryManipulationStrategy->ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_IsComplete() const
    {
    if (!m_geometryManipulationStrategy->IsComplete())
        return false;

    ISolidPrimitivePtr geometry = FinishGeometry();
    if (geometry.IsNull())
        return false;

    DgnExtrusionDetail extDetail;
    if (!geometry->TryGetDgnExtrusionDetail(extDetail))
        return false;

    DPoint3d centroid;
    DVec3d normal;
    double area;
    if (!extDetail.m_baseCurve->CentroidNormalArea(centroid, normal, area))
        return false;
    if (DoubleOps::AlmostEqual(area, 0))
        return false;

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_CanAcceptMorePoints() const
    {
    return m_geometryManipulationStrategy->CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    double & value
) const
    {
    if (0 == strcmp(key, prop_BottomElevation))
        value = m_bottomElevation;
    else if (0 == strcmp(key, prop_TopElevation))
        value = m_topElevation;
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    double const & value
)
    {
    if (0 == strcmp(key, prop_BottomElevation))
        m_bottomElevation = value;
    else if (0 == strcmp(key, prop_TopElevation))
        m_topElevation = value;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    Utf8String & value
) const
    {
    if (0 == strcmp(key, prop_Name))
        {
        if (m_grid.IsValid())
            {
            value = m_grid->GetName();
            return BentleyStatus::SUCCESS;
            }
        }

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    Dgn::DgnElement & value
) const
    {
    /*if (0 == strcmp(key, prop_Axis))
        value = *m_axis.get();
    else if (0 == strcmp(key, prop_Grid))
        value = *m_grid.get();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;*/

    return BentleyStatus::ERROR; // TODO Change DgnElement to DgnElementPtr
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    Dgn::DgnElement const & value
)
    {
    if (0 == strcmp(key, prop_Axis))
        m_axis = dynamic_cast<GridAxisCP>(&value);
    else if (0 == strcmp(key, prop_Grid))
        m_grid = dynamic_cast<GridCP>(&value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::OnDynamicOperationEnd()
    {
    _OnDynamicOperationEnd();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String SketchGridSurfaceManipulationStrategy::GetMessage() const
    {
    return _GetMessage();
    }
