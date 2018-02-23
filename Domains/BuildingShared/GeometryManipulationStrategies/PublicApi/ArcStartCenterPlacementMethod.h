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
    DEFINE_T_SUPER(RefCounted<IArcPlacementMethod>)

    private:
        ArcStartCenterPlacementMethod(ArcManipulationStrategyR manipulationStrategy) : T_Super(manipulationStrategy) {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ArcPlacementMethod _GetMethod() const { return ArcPlacementMethod::StartCenter; }

    public:
        static ArcStartCenterPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcStartCenterPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE