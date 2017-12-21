/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryPlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryPlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryPlacementStrategy : public GeometryManipulationStrategyBase
    {
    DEFINE_T_SUPER(GeometryManipulationStrategyBase)

    private:
        GeometryManipulationStrategyPtr m_manipulationStrategy;

    protected:
        GeometryPlacementStrategy(GeometryManipulationStrategyP manipulationStrategy);

        virtual bvector<DPoint3d> const& _GetKeyPoints() const override;

        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _SetDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index, DynamicKeyPointType type) override;
        virtual void _ResetDynamicKeyPoint() override;

    public:
        void AddKeyPoint(DPoint3dCR newKeyPoint);
        void PopKeyPoint();
    };

END_BUILDING_SHARED_NAMESPACE