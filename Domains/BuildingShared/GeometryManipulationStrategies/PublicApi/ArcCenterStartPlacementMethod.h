/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

        // IArcPlacementMethod
        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _PopKeyPoint() override;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        virtual ArcPlacementMethod _GetMethod() const override { return ArcPlacementMethod::CenterStart; }
        virtual bvector<DPoint3d> _GetKeyPoints() const override;

    public:
        static ArcCenterStartPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcCenterStartPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE