/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurveVectorManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class DefaultNewGeometryType
    {
    Default = 0,
    Line,
    LineString,
    Arc
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
        bvector<CurvePrimitiveManipulationStrategyPtr> m_primitiveStrategies;
        
        DefaultNewGeometryType m_defaultNewGeometryType;

        LinePlacementStrategyType m_defaultLinePlacementStrategyType;
        ArcPlacementStrategyType m_defaultArcPlacementStrategyType;
        LineStringPlacementStrategyType m_defaultLineStringPlacementStrategyType;

        CurvePrimitivePlacementStrategyPtr GetPlacementStrategy(CurvePrimitiveManipulationStrategyR manipulationStrategy) const;
        CurvePrimitivePlacementStrategyPtr GetStrategyForAppend();
        bool IsLastStrategyReadyForPop() const;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorManipulationStrategy();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish() const;

        virtual bvector<DPoint3d> _GetKeyPoints() const override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override { return true; }

        virtual bool _IsDynamicKeyPointSet() const override;
        virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override { BeAssert(false && "Not implemented"); }
        virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _ResetDynamicKeyPoint() override;

        virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override { BeAssert(false && "Not implemented"); }
        virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _PopKeyPoint() override;
        virtual void _RemoveKeyPoint(size_t index) override { BeAssert(false && "Not implemented"); }
        virtual void _Clear() override;

        CV_PROPERTY_OVERRIDE(int)
        CV_PROPERTY_OVERRIDE(double)
        CV_PROPERTY_OVERRIDE(DVec3d)
        CV_PROPERTY_OVERRIDE(DPlane3d)
        CV_PROPERTY_OVERRIDE(Dgn::DgnElementId)
        CV_PROPERTY_OVERRIDE(Dgn::DgnElement)
        CV_PROPERTY_OVERRIDE(Utf8String)
        CV_PROPERTY_OVERRIDE(bvector<double>)
        CV_PROPERTY_OVERRIDE(bvector<Utf8String>)
        CV_PROPERTY_OVERRIDE(GeometryManipulationStrategyProperty)

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPtr Finish() const;

        static CurveVectorManipulationStrategyPtr Create() { return new CurveVectorManipulationStrategy(); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultNewGeometryType(DefaultNewGeometryType newGeometryType);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(LinePlacementStrategyType newPlacementStrategyType);
        void ChangeDefaultPlacementStrategy(ArcPlacementStrategyType newPlacementStrategyType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType newPlacementStrategyType);
    };

END_BUILDING_SHARED_NAMESPACE
