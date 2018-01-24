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

    private:
        BBS::ExtrusionManipulationStrategyPtr m_geometryManipulationStrategy;
        double m_bottomElevation = 0;
        double m_topElevation = 0;
        GridAxisCPtr m_axis = nullptr; // TODO to Ptr
        GridCPtr m_grid = nullptr; // TODO to Ptr

        ISolidPrimitivePtr FinishGeometry() const;

    protected:
        SketchGridSurfaceManipulationStrategy()
            : SketchGridSurfaceManipulationStrategy(*BBS::ExtrusionManipulationStrategy::Create()) { }

        SketchGridSurfaceManipulationStrategy(BBS::ExtrusionManipulationStrategyR geometryManipulationStrategy);

        virtual BBS::GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryManipulationStrategyR _GetGeometryManipulationStrategyR() override { return *m_geometryManipulationStrategy; }

        virtual bvector<DPoint3d> _GetKeyPoints() const override;

        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _ResetDynamicKeyPoint() override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;

        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
        virtual void _SetProperty(Utf8CP key, double const & value) override;

        virtual BentleyStatus _TryGetProperty(Utf8CP key, Utf8String& value) const override;

        virtual BentleyStatus _TryGetProperty(Utf8CP key, Dgn::DgnElement& value) const override;
        virtual void _SetProperty(Utf8CP key, Dgn::DgnElement const& value) override;

        virtual void _OnDynamicOperationEnd() = 0;
        virtual Utf8String _GetMessage() const = 0;

    public:
        void OnDynamicOperationEnd();
        Utf8String GetMessage() const;

        static const Utf8CP prop_BottomElevation;
        static const Utf8CP prop_TopElevation;
        static const Utf8CP prop_Axis;
        static const Utf8CP prop_Name;
        static const Utf8CP prop_Grid;
    };

END_GRIDS_NAMESPACE