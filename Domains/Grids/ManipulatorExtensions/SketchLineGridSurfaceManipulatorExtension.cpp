/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/ManipulatorExtensions/SketchLineGridSurfaceManipulatorExtension.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/GridManipulatorsAPI.h"
#include <DgnView/DgnTool.h>
#include <DgnView/DragManipulatorBase.h>

USING_NAMESPACE_GRIDS

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct SketchLineGridSurfaceDragManipulator : Dgn::PointDragManipulator
    {
    DEFINE_T_SUPER(Dgn::PointDragManipulator)

    struct ControlPoint : Dgn::PointDragManipulator::ControlPoint
        {
        DEFINE_T_SUPER(Dgn::PointDragManipulator::ControlPoint)

        private:
            LineGridSurfaceManipulationStrategyR m_strategy;
            size_t m_keyPointIndex;

        public:
            ControlPoint(LineGridSurfaceManipulationStrategyR strategy, size_t keyPointIndex, DPoint3dCR point)
                : T_Super(point)
                , m_keyPointIndex(keyPointIndex)
                , m_strategy(strategy)
                {}

            void UpdatePoint(DPoint3dCR newPoint, bool isDynamics);
        };

    private:
        SketchLineGridSurfacePtr m_surface;
        LineGridSurfaceManipulationStrategyPtr m_strategy;

    protected:
        SketchLineGridSurfaceDragManipulator(SketchLineGridSurfaceR element);

        bool _IsDisplayedInView(Dgn::DgnViewportR vp) override;
        StatusInt _DoModify(Dgn::DgnButtonEventCR ev, bool isDynamics) override;
        bool _DoCreateControls() override;
        StatusInt _OnModifyAccept(Dgn::DgnButtonEventCR ev) override;

    public:
        static Dgn::IEditManipulatorPtr Create(SketchLineGridSurfaceR element);
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchLineGridSurfaceDragManipulator::ControlPoint::UpdatePoint
(
    DPoint3dCR newPoint,
    bool isDynamics
)
    {
    if (isDynamics)
        m_strategy.UpdateDynamicKeyPoint(newPoint, m_keyPointIndex);
    else
        m_strategy.ReplaceKeyPoint(newPoint, m_keyPointIndex);

    m_strategy.FinishElement();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchLineGridSurfaceDragManipulator::_IsDisplayedInView
(
    Dgn::DgnViewportR vp
)
    {
    if (!vp.GetViewController().IsModelViewed(m_surface->GetModelId()))
        return false;

    if (!vp.GetViewController().GetViewedCategories().Contains(m_surface->GetCategoryId()))
        return false;

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
StatusInt SketchLineGridSurfaceDragManipulator::_OnModifyAccept
(
    Dgn::DgnButtonEventCR ev
)
    {
    StatusInt status = T_Super::_OnModifyAccept(ev);
    if (SUCCESS != status)
        return status;

    if (m_surface.IsNull())
        return ERROR;

    return m_surface->GetDgnDb().SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
StatusInt SketchLineGridSurfaceDragManipulator::_DoModify
(
    Dgn::DgnButtonEventCR ev, 
    bool isDynamics
)
    {
    ControlPoint* controlPoint = dynamic_cast<ControlPoint*>(m_controls.m_locations[m_controls.FindFirstSelected()]);
    if (nullptr == controlPoint)
        return ERROR;

    DPoint3dCP point = ev.GetPoint();
    if (nullptr == point)
        return ERROR;

    controlPoint->UpdatePoint(*point, isDynamics);

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
SketchLineGridSurfaceDragManipulator::SketchLineGridSurfaceDragManipulator
(
    SketchLineGridSurfaceR element
)
    : m_surface(&element)
    , m_strategy(LineGridSurfaceManipulationStrategy::Create(element))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchLineGridSurfaceDragManipulator::_DoCreateControls()
    {
    m_controls.ClearControls();
    
    if (m_strategy.IsNull())
        return false;

    bvector<DPoint3d> const& keyPoints = m_strategy->GetKeyPoints();
    for (size_t index = 0; index < keyPoints.size(); ++index)
        {
        m_controls.m_locations.push_back(new ControlPoint(*m_strategy, index, keyPoints[index]));
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IEditManipulatorPtr SketchLineGridSurfaceDragManipulator::Create
(
    SketchLineGridSurfaceR element
)
    {
    return new SketchLineGridSurfaceDragManipulator(element);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IEditManipulatorPtr SketchLineGridSurfaceManipulatorExtension::GetIEditManipulator
(
    Dgn::DgnElementCR element
)
    {
    SketchLineGridSurfacePtr surface = SketchLineGridSurface::GetForEdit(element.GetDgnDb(), element.GetElementId());
    if (surface.IsNull())
        return nullptr;

    return SketchLineGridSurfaceDragManipulator::Create(*surface);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IEditManipulatorPtr SketchLineGridSurfaceManipulatorExtension::_GetIEditManipulator
(
    Dgn::GeometrySourceCR gelm
)
    {
    Dgn::DgnElementCPtr elem = gelm.ToElement();
    if (elem.IsNull())
        return nullptr;

    return GetIEditManipulator(*elem);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IEditManipulatorPtr SketchLineGridSurfaceManipulatorExtension::_GetIEditManipulator
(
    Dgn::HitDetailCR hit
)
    {
    Dgn::DgnElementCPtr elem = hit.GetElement();
    if (elem.IsNull())
        return nullptr;

    return GetIEditManipulator(*elem);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchLineGridSurfaceManipulatorExtension::RegisterDragManipulatorExtensions()
    {
    static SketchLineGridSurfaceManipulatorExtension instance;
    IEditManipulatorExtension::RegisterExtension(SketchLineGridSurfaceHandler::GetHandler(), instance);
    }