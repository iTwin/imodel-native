/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/NullGeometryPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct NullGeometryPlacementStrategy final : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    private:
        NullGeometryManipulationStrategyPtr m_manipulationStrategy;

        NullGeometryPlacementStrategy() 
            : T_Super()
            , m_manipulationStrategy(NullGeometryManipulationStrategy::Create())
            {}

    protected:
        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

        virtual bvector<DPoint3d> _GetKeyPoints() const override { return bvector<DPoint3d>(); }

        virtual bool _IsDynamicKeyPointSet() const override { return false; }
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override {}
        virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override {}
        virtual void _ResetDynamicKeyPoint() override {}

        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override {}
        virtual void _PopKeyPoint() override {}

        virtual bool _IsComplete() const override { return false; }
        virtual bool _CanAcceptMorePoints() const override { return false; }

    public:
        static NullGeometryPlacementStrategyPtr Create() { return new NullGeometryPlacementStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE