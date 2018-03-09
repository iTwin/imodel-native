/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcStartMidEndPlacementMethod.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcStartMidEndPlacementMethod : RefCounted<IArcPlacementMethod>
    {
    private:
        ArcManipulationStrategyR m_manipulationStrategy;

        ArcStartMidEndPlacementMethod(ArcManipulationStrategyR manipulationStrategy) 
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
        virtual ArcPlacementMethod _GetMethod() const override { return ArcPlacementMethod::StartMidEnd; }
        virtual bvector<DPoint3d> _GetKeyPoints() const override;

    public:
        static ArcStartMidEndPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcStartMidEndPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE