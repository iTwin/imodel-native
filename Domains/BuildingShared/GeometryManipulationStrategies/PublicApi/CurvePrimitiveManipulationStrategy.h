/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurvePrimitiveManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurvePrimitiveManipulationStrategy : public GeometryManipulationStrategy
    {
    DEFINE_T_SUPER(GeometryManipulationStrategy)

    private:
        bvector<DPoint3d> m_keyPoints;
        bvector<DPoint3d> m_keyPointsWithDynamicKeyPoint;
        bool m_dynamicKeyPointSet;

    protected:
        CurvePrimitiveManipulationStrategy() : T_Super(), m_dynamicKeyPointSet(false) {}

        bvector<DPoint3d>& GetKeyPointsR() { return m_keyPoints; }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> _GetKeyPoints() const override;

        virtual bool _IsDynamicKeyPointSet() const override { return m_dynamicKeyPointSet; }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _RemoveKeyPoint(size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _Clear() override;

        virtual ICurvePrimitivePtr _FinishPrimitive() const = 0;

        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() = 0;
        virtual LinePlacementStrategyPtr _CreateLinePlacementStrategy(LinePlacementStrategyType strategyType) { BeAssert(false && "This is not a LineManipulationStrategy"); return nullptr; }
        virtual ArcPlacementStrategyPtr _CreateArcPlacementStrategy(ArcPlacementStrategyType strategyType) { BeAssert(false && "This is not an ArcManipulationStrategy"); return nullptr; }
        virtual LineStringPlacementStrategyPtr _CreateLineStringPlacementStrategy(LineStringPlacementStrategyType strategyType) { BeAssert(false && "This is not a LineStringManipulationStrategy"); return nullptr; }
        virtual SplinePlacementStrategyPtr _CreateSplinePlacementStrategy(SplinePlacementStrategyType strategyType) { BeAssert(false && "This is not a SplineControlPointsManipulationStrategy"); return nullptr; }
        
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsEmpty() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsSingleKeyPointLeft() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DPoint3d _GetLastKeyPoint() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DPoint3d _GetFirstKeyPoint() const;

        virtual CurvePrimitiveManipulationStrategyPtr _Clone() const = 0;

        virtual bool _IsContinious() const = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ICurvePrimitivePtr FinishPrimitive() const;
        
        CurvePrimitivePlacementStrategyPtr CreateDefaultPlacementStrategy();
        LinePlacementStrategyPtr CreateLinePlacementStrategy(LinePlacementStrategyType strategyType);
        ArcPlacementStrategyPtr CreateArcPlacementStrategy(ArcPlacementStrategyType strategyType);
        LineStringPlacementStrategyPtr CreateLineStringPlacementStrategy(LineStringPlacementStrategyType strategyType);
        SplinePlacementStrategyPtr CreateSplinePlacementStrategy(SplinePlacementStrategyType strategyType);
        
        bvector<DPoint3d> const& GetAcceptedKeyPoints() const { return m_keyPoints; }

        bool IsEmpty() const;
        bool IsSingleKeyPointLeft() const;
        DPoint3d GetLastKeyPoint() const;
        DPoint3d GetFirstKeyPoint() const;

        CurvePrimitiveManipulationStrategyPtr Clone() const;

        bool IsContinious() const;
    };

END_BUILDING_SHARED_NAMESPACE