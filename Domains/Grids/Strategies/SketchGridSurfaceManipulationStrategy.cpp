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
BentleyStatus SketchGridSurfaceManipulationStrategy::_UpdateGridSurface()
    {
    PlanGridPlanarSurfacePtr surface = _GetGridSurfaceP();

    if (m_axis.IsNull() || surface.IsNull())
        return BentleyStatus::ERROR;

    if (m_axis->GetElementId() != surface->GetAxisId())
        surface->SetAxisId(m_axis->GetElementId());

    if (surface->GetStartElevation() != m_bottomElevation)
        surface->SetStartElevation(m_bottomElevation);
    
    if (surface->GetEndElevation() != m_topElevation)
        surface->SetEndElevation(m_topElevation);

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr SketchGridSurfaceManipulationStrategy::_FinishElement()
    {
    PlanGridPlanarSurfacePtr surface = _GetGridSurfaceP();
    BeAssert(surface.IsValid() && "Shouldn't be called with invalid surface");
    
    if (surface.IsNull() || !IsComplete())
        return nullptr;

    if (BentleyStatus::ERROR == _UpdateGridSurface())
        return nullptr;

    if (!surface->GetDgnDb().Txns().InDynamicTxn())
        if (Dgn::RepositoryStatus::Success != BuildingLocks_LockElementForOperation(*surface, BeSQLite::DbOpcode::Update, "Update Grid Surface"))
            return nullptr;

    if (surface->Update().IsNull())
        return nullptr;

    return surface;
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
        return _GetGeometryPlacementStrategy()->TryGetProperty(key, value);

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
SketchGridSurfaceManipulationStrategy::SketchGridSurfaceManipulationStrategy()
    : T_Super()
    , m_axis(nullptr)
    , m_gridName("")
    , m_bottomElevation(0)
    , m_topElevation(0)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> SketchGridSurfaceManipulationStrategy::_GetKeyPoints() const
    {
    return _GetGeometryPlacementStrategy()->GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    return _GetGeometryPlacementStrategy()->IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_ResetDynamicKeyPoint()
    {
    _GetGeometryPlacementStrategyP()->ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_IsComplete() const
    {
    if (_GetGeometryPlacementStrategy()->IsComplete())
        return false;

    if (m_axis.IsNull() && m_gridName.empty())
        return false;

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_CanAcceptMorePoints() const
    {
    return _GetGeometryPlacementStrategy()->CanAcceptMorePoints();
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
    else
        _GetGeometryPlacementStrategyP()->SetProperty(key, value);
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
        value = m_gridName;
        return BentleyStatus::SUCCESS;
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
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    Utf8String const & value
)
    {
    if (0 == strcmp(key, prop_Name))
        {
        m_gridName = value;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String SketchGridSurfaceManipulationStrategy::GetMessage() const
    {
    return _GetMessage();
    }
