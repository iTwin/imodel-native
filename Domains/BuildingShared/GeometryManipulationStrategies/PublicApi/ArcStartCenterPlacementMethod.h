/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcStartCenterPlacementMethod.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcStartCenterPlacementMethod : RefCounted<IArcPlacementMethod>
    {
    private:
        ArcManipulationStrategyR m_manipulationStrategy;

        ArcStartCenterPlacementMethod(ArcManipulationStrategyR manipulationStrategy) 
            : m_manipulationStrategy(manipulationStrategy)
            {}

    protected:
        ArcManipulationStrategyCR GetArcManipulationStrategy() const { return m_manipulationStrategy; }
        ArcManipulationStrategyR GetArcManipulationStrategyForEdit() { return m_manipulationStrategy; }

        //IArcPlacementMethod
        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _PopKeyPoint() override;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override { BeAssert(false && "Not implemented"); }
        virtual ArcPlacementMethod _GetMethod() const override { return ArcPlacementMethod::StartCenter; }
        virtual bvector<DPoint3d> _GetKeyPoints() const override;

    public:
        static ArcStartCenterPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcStartCenterPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE