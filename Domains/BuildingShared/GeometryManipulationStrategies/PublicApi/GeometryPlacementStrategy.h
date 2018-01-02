/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    protected:
        GeometryManipulationStrategyPtr m_manipulationStrategy;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryPlacementStrategy(GeometryManipulationStrategyP manipulationStrategy);

        GeometryManipulationStrategyR GetManipulationStrategyR() { return *m_manipulationStrategy; }
        GeometryManipulationStrategyCR GetManipulationStrategy() const { return *m_manipulationStrategy; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> const& _GetKeyPoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsDynamicKeyPointSet() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint();

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
    };

END_BUILDING_SHARED_NAMESPACE