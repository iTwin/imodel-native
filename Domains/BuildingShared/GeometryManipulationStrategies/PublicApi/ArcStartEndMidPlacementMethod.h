/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcStartEndMidPlacementMethod.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcStartEndMidPlacementMethod : RefCounted<IArcPlacementMethod>
    {
    DEFINE_T_SUPER(RefCounted<IArcPlacementMethod>)

    private:
        ArcStartEndMidPlacementMethod(ArcManipulationStrategyR manipulationStrategy)
            : T_Super(manipulationStrategy)
            {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ArcPlacementMethod _GetMethod() const { return ArcPlacementMethod::StartEndMid; }
        virtual bvector<DPoint3d> _GetKeyPoints() const override;

    public:
        static ArcStartEndMidPlacementMethodPtr Create(ArcManipulationStrategyR manipulationStrategy) { return new ArcStartEndMidPlacementMethod(manipulationStrategy); }
    };

END_BUILDING_SHARED_NAMESPACE