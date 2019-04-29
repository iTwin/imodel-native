/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

        // IRessetableDynamic
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetDynamicState(DynamicStateBaseCR state) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DynamicStateBaseCPtr _GetDynamicState() const override;

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
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual IGeometryPtr _FinishGeometry() const override;

        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() = 0;
        virtual LinePlacementStrategyPtr _CreateLinePlacementStrategy(LinePlacementStrategyType strategyType) { BeAssert(false && "This is not a LineManipulationStrategy"); return nullptr; }
        virtual ArcPlacementStrategyPtr _CreateArcPlacementStrategy(ArcPlacementMethod method) { BeAssert(false && "This is not an ArcManipulationStrategy"); return nullptr; }
        virtual LineStringPlacementStrategyPtr _CreateLineStringPlacementStrategy(LineStringPlacementStrategyType strategyType) { BeAssert(false && "This is not a LineStringManipulationStrategy"); return nullptr; }
        virtual SplinePlacementStrategyPtr _CreateSplinePlacementStrategy(SplinePlacementStrategyType strategyType) { BeAssert(false && "This is not a SplineControlPointsManipulationStrategy"); return nullptr; }
        
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsEmpty() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsSingleKeyPointLeft() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DPoint3d _GetLastKeyPoint() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DPoint3d _GetFirstKeyPoint() const;

        virtual CurvePrimitiveManipulationStrategyPtr _Clone() const = 0;

        virtual bool _IsContinious() const = 0;

        virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ICurvePrimitivePtr FinishPrimitive() const;
        
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitivePlacementStrategyPtr CreateDefaultPlacementStrategy();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePlacementStrategyPtr CreateLinePlacementStrategy(LinePlacementStrategyType strategyType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcPlacementStrategyPtr CreateArcPlacementStrategy(ArcPlacementMethod strategyType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LineStringPlacementStrategyPtr CreateLineStringPlacementStrategy(LineStringPlacementStrategyType strategyType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT SplinePlacementStrategyPtr CreateSplinePlacementStrategy(SplinePlacementStrategyType strategyType);
        
        bvector<DPoint3d> const& GetAcceptedKeyPoints() const { return m_keyPoints; }

        bool IsEmpty() const;
        bool IsSingleKeyPointLeft() const;
        DPoint3d GetLastKeyPoint() const;
        DPoint3d GetFirstKeyPoint() const;

        CurvePrimitiveManipulationStrategyPtr Clone() const;

        bool IsContinious() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ICurvePrimitive::CurvePrimitiveType GetResultCurvePrimitiveType() const;
    };

END_BUILDING_SHARED_NAMESPACE