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
    DEFINE_T_SUPER(RefCounted<IArcPlacementMethod>)

    private:
        ArcCenterStartPlacementMethod(ArcManipulationStrategyR manipulationStrategy) : T_Super(manipulationStrategy) {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ArcPlacementMethod _GetMethod() const { return ArcPlacementMethod::CenterStart; }
        virtual bvector<DPoint3d> _GetKeyPoints() const override;

    public:
        static ArcCenterStartPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcCenterStartPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE