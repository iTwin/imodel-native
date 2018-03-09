/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcCenterStartPlacementMethod.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcCenterStartPlacementMethod : RefCounted<IArcPlacementMethod>
    {
    private:
        ArcManipulationStrategyR m_manipulationStrategy;

        ArcCenterStartPlacementMethod(ArcManipulationStrategyR manipulationStrategy)
            : m_manipulationStrategy(manipulationStrategy)
            {}

    protected:
        ArcManipulationStrategyCR GetArcManipulationStrategy() const { return m_manipulationStrategy; }
        ArcManipulationStrategyR GetArcManipulationStrategyForEdit() { return m_manipulationStrategy; }

    public:
        // IArcPlacementMethod
        void AddKeyPoint(DPoint3dCR newKeyPoint) override;
        void PopKeyPoint() override;
        void AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        void AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        ArcPlacementMethod GetMethod() const override { return ArcPlacementMethod::CenterStart; }
        bvector<DPoint3d> GetKeyPoints() const override;

        static ArcCenterStartPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcCenterStartPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE