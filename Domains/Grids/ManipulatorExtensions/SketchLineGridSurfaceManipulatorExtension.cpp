/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/GridManipulatorsAPI.h"

USING_NAMESPACE_GRIDS
USING_NAMESPACE_BUILDING_SHARED

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct SketchLineGridSurfaceDragManipulator : GeometricElementManipulator
    {
    DEFINE_T_SUPER(GeometricElementManipulator)

    private:
        SketchLineGridSurfacePtr m_surface;
        LineGridSurfaceManipulationStrategyPtr m_strategy;

    protected:
        SketchLineGridSurfaceDragManipulator(SketchLineGridSurfaceR element);

        virtual DgnElementManipulationStrategyR _GetStrategy() override { return *m_strategy; }
        virtual Dgn::GeometricElementR _GetElement() override { return *m_surface; }

    public:
        static Dgn::IEditManipulatorPtr Create(SketchLineGridSurfaceR element);
    };

END_GRIDS_NAMESPACE

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
    BeAssert(m_surface.IsValid());
    BeAssert(m_strategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IEditManipulatorPtr SketchLineGridSurfaceDragManipulator::Create
(
    SketchLineGridSurfaceR element
)
    {
    SketchLineGridSurfaceDragManipulator* manipulator = new SketchLineGridSurfaceDragManipulator(element);

    if (manipulator->m_surface.IsNull() || manipulator->m_strategy.IsNull())
        {
        delete manipulator;
        return nullptr;
        }

    return manipulator;
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