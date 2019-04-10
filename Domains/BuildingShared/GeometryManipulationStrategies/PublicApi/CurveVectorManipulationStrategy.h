/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurveVectorManipulationStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class DefaultNewGeometryType
    {
    Default = 0,
    Line,
    LineString,
    Arc,
    Spline,             //! ControlPoints
    InterpolationCurve  //! ThroughPoints
    };

typedef bpair<size_t, size_t> PrimitiveStrategyKeyPointIndexRange;
typedef bpair<CurvePrimitiveManipulationStrategyPtr, PrimitiveStrategyKeyPointIndexRange> PrimitiveStrategyWithKeyPointIndexRange;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct CurvePrimitiveStrategyContainer
    {
    private:
        bvector<CurvePrimitiveManipulationStrategyPtr> m_primitiveManipulationStrategies;
        bvector<CurvePrimitivePlacementStrategyPtr> m_primitivePlacementStrategies;

        DefaultNewGeometryType m_defaultNewGeometryType;
        LinePlacementStrategyType m_defaultLinePlacementStrategyType;
        ArcPlacementMethod m_defaultArcPlacementMethod;
        LineStringPlacementStrategyType m_defaultLineStringPlacementStrategyType;

        void CreateNext(CurvePrimitiveManipulationStrategyPtr&, CurvePrimitivePlacementStrategyPtr&) const;
        void PrepareNextStrategyAfterTypeOrPlacementChange();

    public:
        CurvePrimitiveStrategyContainer();

        void SetDefaultNewGeometryType(DefaultNewGeometryType);
        void SetDefaultPlacementStrategy(LinePlacementStrategyType);
        void SetDefaultPlacementStrategy(ArcPlacementMethod);
        void SetDefaultPlacementStrategy(LineStringPlacementStrategyType);

        void Clear();
        void Pop();
        void AddNext();
        void Append(CurvePrimitiveManipulationStrategyR);
        bool IsEmpty() const;
        bool IsComplete() const;
        bool CanAcceptMorePoints() const;
        void ResetDynamicKeyPoint();
        
        bvector<PrimitiveStrategyWithKeyPointIndexRange> GetStrategies(size_t keyPointIndex) const;
        bvector<CurvePrimitiveManipulationStrategyPtr> const& GetManipulationStrategies() const { return m_primitiveManipulationStrategies; }
        bvector<CurvePrimitivePlacementStrategyPtr> const& GetPlacementStrategies() const { return m_primitivePlacementStrategies; }
    };

#define CV_PROPERTY_OVERRIDE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, value_type const& value) override; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const override;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurveVectorManipulationStrategy : public GeometryManipulationStrategy
    {
    DEFINE_T_SUPER(GeometryManipulationStrategy)

    private:
        CurvePrimitiveStrategyContainer m_primitiveStrategyContainer;
       
        DPlane3d m_workingPlane;

        CurvePrimitivePlacementStrategyR GetStrategyForAppend();
        CurvePrimitivePlacementStrategyR GetStrategyForSetProperty();
        bool IsLastStrategyReadyForPop() const;
        void ConnectStartEnd(CurveVectorR cv) const;

        template <typename T> void UpdateKeyPoint(size_t index, T updateFn);

        bvector<PrimitiveStrategyWithKeyPointIndexRange> GetPrimitiveStrategies(size_t index) const;

        friend struct CurveVectorPlacementStrategy;
    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorManipulationStrategy();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish(bool connectEndStart) const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> _GetKeyPoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override { return true; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsDynamicKeyPointSet() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override { BeAssert(false && "Not implemented"); }
        virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;
        virtual void _RemoveKeyPoint(size_t index) override { BeAssert(false && "Not implemented"); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _Clear() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DPoint3d _AdjustPoint(DPoint3d keyPoint) const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual IGeometryPtr _FinishGeometry() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual LineStringManipulationStrategyPtr _InitLineStringManipulationStrategy(ICurvePrimitiveCR) const;
        virtual ChildCurveVectorManipulationStrategyPtr _InitChildCurveVectorManipulationStrategy(ICurvePrimitiveCR) const;
        virtual ArcManipulationStrategyPtr _InitArcManipulationStrategy(ICurvePrimitiveCR) const;

        // IRessetableDynamic
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetDynamicState(DynamicStateBaseCR state) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DynamicStateBaseCPtr _GetDynamicState() const override;

        CV_PROPERTY_OVERRIDE(bool)
        CV_PROPERTY_OVERRIDE(int)
        CV_PROPERTY_OVERRIDE(double)
        CV_PROPERTY_OVERRIDE(DVec3d)
        CV_PROPERTY_OVERRIDE(DPlane3d)
        CV_PROPERTY_OVERRIDE(RotMatrix)
        CV_PROPERTY_OVERRIDE(Utf8String)
        CV_PROPERTY_OVERRIDE(bvector<double>)
        CV_PROPERTY_OVERRIDE(bvector<Utf8String>)
        CV_PROPERTY_OVERRIDE(GeometryManipulationStrategyProperty)

    public:
        static constexpr Utf8CP prop_WorkingPlane() { return "WorkingPlane"; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPtr Finish(bool connectEndStart = false) const;

        static CurveVectorManipulationStrategyPtr Create() { return new CurveVectorManipulationStrategy(); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static CurveVectorManipulationStrategyPtr Create(CurveVectorCR cv);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void Init(CurveVectorCR cv);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultNewGeometryType(DefaultNewGeometryType newGeometryType);

        bool FinishContiniousPrimitive();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(LinePlacementStrategyType newPlacementStrategyType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(ArcPlacementMethod method);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType newPlacementStrategyType);

        DPlane3dCR GetWorkingPlane() const { return m_workingPlane; }
    };

END_BUILDING_SHARED_NAMESPACE
