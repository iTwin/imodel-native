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
        PlanGridPlanarSurfacePtr m_surface;

        SketchGridSurfaceManipulationStrategy();

        // GeometryManipulationStrategyBase
        virtual bvector<DPoint3d> _GetKeyPoints() const override;
        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _ResetDynamicKeyPoint() override;
        virtual bool _IsComplete() const override;
        virtual void _SetProperty(Utf8CP key, double const & value) override;
        virtual void _SetProperty(Utf8CP key, Dgn::DgnElement const& value) override;
        virtual void _SetProperty(Utf8CP key, Utf8String const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, Dgn::DgnElement & value) const override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, Utf8String & value) const override;

        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement() override;

        // SketchGridSurfaceManipulationStrategy
        virtual BentleyStatus _UpdateGridSurface();
        virtual Utf8String _GetMessage() const = 0;
        virtual BBS::CurvePrimitivePlacementStrategyPtr _GetStrategyForAppend() = 0;

    public:
        Utf8String GetMessage() const;

        static const Utf8CP prop_BottomElevation;
        static const Utf8CP prop_TopElevation;
        static const Utf8CP prop_Axis;
        static const Utf8CP prop_Name;
        static const Utf8CP prop_Grid;
    };

END_GRIDS_NAMESPACE