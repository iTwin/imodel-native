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
        virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override;
        virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override;
        virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        virtual bvector<DPoint3d> _GetKeyPoints() const override;
        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _ResetDynamicKeyPoint() override;
        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;
        virtual void _SetProperty(Utf8CP key, double const & value) override;
        virtual void _SetProperty(Utf8CP key, Dgn::DgnElement const& value) override;
        virtual void _SetProperty(Utf8CP key, Utf8String const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, Dgn::DgnElement & value) const override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, Utf8String & value) const override;

        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement() override;

        // SketchGridSurfaceManipulationStrategy
        DPoint3d ProjectPoint(DPoint3d point);
        BentleyStatus GetOrCreateGridAndAxis(SketchGridCPtr& grid, Dgn::SpatialLocationModelPtr spatialModel);
        void ProjectExistingPoints();
        virtual BentleyStatus _UpdateGridSurface();
        virtual Utf8String _GetMessage() const = 0;
        virtual PlanGridPlanarSurfaceCP _GetGridSurfaceCP() = 0;
        virtual PlanGridPlanarSurfaceP _GetGridSurfaceP() = 0;
        virtual BBS::CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const = 0;
        virtual BBS::CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyR() = 0;
        virtual BBS::CurvePrimitivePlacementStrategyPtr _GetGeometryPlacementStrategyP() = 0;
        virtual BBS::CurvePrimitivePlacementStrategyCPtr _GetGeometryPlacementStrategy() const = 0;

    public:
        Utf8String GetMessage() const;

        static const Utf8CP prop_BottomElevation;
        static const Utf8CP prop_TopElevation;
        static const Utf8CP prop_Axis;
        static const Utf8CP prop_Name;
    };

END_GRIDS_NAMESPACE