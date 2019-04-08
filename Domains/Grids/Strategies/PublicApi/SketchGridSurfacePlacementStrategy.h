/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/SketchGridSurfacePlacementStrategy.h $
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
struct SketchGridSurfacePlacementStrategy : public BBS::DgnElementPlacementStrategy
    {
    DEFINE_T_SUPER(BBS::DgnElementPlacementStrategy)
        
    protected:
        SketchGridSurfacePlacementStrategy(Dgn::DgnDbR db) : T_Super(db) {}

        virtual BBS::DgnElementManipulationStrategyCR _GetDgnElementManipulationStrategy() const override { return _GetSketchGridSurfaceManipulationStrategy(); }
        virtual BBS::DgnElementManipulationStrategyR _GetDgnElementManipulationStrategyForEdit() override { return _GetSketchGridSurfaceManipulationStrategyForEdit(); }
        virtual BBS::GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return _GetSketchGridSurfaceManipulationStrategy(); }
        virtual BBS::GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return _GetSketchGridSurfaceManipulationStrategyForEdit(); }

        virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const = 0;
        virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyForEdit() = 0;

        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint);

        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
        virtual void _SetProperty(Utf8CP key, double const & value) override;

        virtual BentleyStatus _TryGetProperty(Utf8CP key, Utf8String& value) const override;

        virtual BentleyStatus _TryGetProperty(Utf8CP key, Dgn::DgnElementCP& value) const override;
        virtual void _SetProperty(Utf8CP key, Dgn::DgnElementCP const& value) override;

        virtual Utf8String _GetMessage() const override;
        virtual void _AddViewOverlay(Dgn::Render::GraphicBuilderR builder, DRange3dCR viewRange, TransformCR worldToView, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const;

    public:        
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_BottomElevation;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_TopElevation;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_Axis;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_Name;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_WorkingPlane;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_Length;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_Angle;
    };

END_GRIDS_NAMESPACE