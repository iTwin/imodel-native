/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/SketchGridSurfaceManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SketchGridSurfaceManipulationStrategy : public BBS::ElementManipulationStrategy
    {
    DEFINE_T_SUPER(BBS::ElementManipulationStrategy)

    protected:
        double m_bottomElevation;
        double m_topElevation;
        GridAxisCPtr m_axis; // TODO to Ptr
        Utf8String m_gridName;
        DPlane3d m_workingPlane;

        SketchGridSurfaceManipulationStrategy();

        // GeometryManipulationStrategyBase
        virtual bvector<DPoint3d> _GetKeyPoints() const override;
        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _ResetDynamicKeyPoint() override;
        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;
        virtual void _SetProperty(Utf8CP key, double const & value) override;
        virtual void _SetProperty(Utf8CP key, Dgn::DgnElementCP const& value) override;
        virtual void _SetProperty(Utf8CP key, Utf8String const& value) override;
        virtual void _SetProperty(Utf8CP key, DPlane3d const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, Dgn::DgnElementCP & value) const override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, Utf8String & value) const override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, DPlane3d & value) const override;

        // GeometryManipulationStrategy
        virtual DPoint3d _AdjustPoint(DPoint3d point) const override;

        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement() override;

        // SketchGridSurfaceManipulationStrategy
        DPoint3d TransformPointBetweenPlanes(DPoint3d const& point, DPlane3d const & from, DPlane3d const & to);
        BentleyStatus GetOrCreateGridAndAxis(SketchGridCPtr& grid, Dgn::SpatialLocationModelPtr spatialModel);
        virtual void _OnWorkingPlaneChanged(DPlane3d const & original);
        virtual BentleyStatus _UpdateGridSurface();
        virtual Utf8String _GetMessage() const = 0;
        virtual PlanGridPlanarSurfaceCP _GetGridSurfaceCP() = 0;
        virtual PlanGridPlanarSurfaceP _GetGridSurfaceP() = 0;
        virtual BBS::CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const = 0;
        virtual BBS::CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() = 0;

        void TransformPointsOnXYPlane(bvector<DPoint3d>& points);
    public:
        Utf8String GetMessage() const;

        static const Utf8CP prop_BottomElevation;
        static const Utf8CP prop_TopElevation;
        static const Utf8CP prop_Axis;
        static const Utf8CP prop_Name;
        static const Utf8CP prop_WorkingPlane;
        static const Utf8CP prop_Length;
        static const Utf8CP prop_Angle;
    };

END_GRIDS_NAMESPACE