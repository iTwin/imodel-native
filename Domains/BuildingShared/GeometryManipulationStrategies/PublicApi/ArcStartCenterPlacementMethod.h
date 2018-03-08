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

    public:
        //IArcPlacementMethod
        void AddKeyPoint(DPoint3dCR newKeyPoint);
        void PopKeyPoint();
        void AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        void AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) { BeAssert(false && "Not implemented"); }
        ArcPlacementMethod GetMethod() const { return ArcPlacementMethod::StartCenter; }
        bvector<DPoint3d> GetKeyPoints() const;

        static ArcStartCenterPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcStartCenterPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE