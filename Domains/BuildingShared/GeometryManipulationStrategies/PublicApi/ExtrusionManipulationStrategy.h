/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct BaseShapeStrategyChangeProperty : public GeometryManipulationStrategyProperty
    {
    DEFINE_T_SUPER(GeometryManipulationStrategyProperty)

    private:
        DefaultNewGeometryType m_defaultNewGeometryType;
        bool m_defaultNewGeometryTypeSet = false;

        LinePlacementStrategyType m_defaultLinePlacementStrategyType;
        bool m_defaultLinePlacementStrategyTypeSet = false;

        ArcPlacementMethod m_defaultArcPlacementMethod;
        bool m_defaultArcPlacementMethodSet = false;

        LineStringPlacementStrategyType m_defaultLineStringPlacementStrategyType;
        bool m_defaultLineStringPlacementStrategyTypeSet = false;

    public:
        BaseShapeStrategyChangeProperty(DefaultNewGeometryType newGeometryType)
            : T_Super()
            , m_defaultNewGeometryType(newGeometryType)
            , m_defaultNewGeometryTypeSet(true)
            {}
        BaseShapeStrategyChangeProperty(LinePlacementStrategyType linePlacementStrategyType)
            : T_Super()
            , m_defaultLinePlacementStrategyType(linePlacementStrategyType)
            , m_defaultLinePlacementStrategyTypeSet(true)
            {}
        BaseShapeStrategyChangeProperty(ArcPlacementMethod method)
            : T_Super()
            , m_defaultArcPlacementMethod(method)
            , m_defaultArcPlacementMethodSet(true)
            {}
        BaseShapeStrategyChangeProperty(LineStringPlacementStrategyType lineStringPlacementStrategyType)
            : T_Super()
            , m_defaultLineStringPlacementStrategyType(lineStringPlacementStrategyType)
            , m_defaultLineStringPlacementStrategyTypeSet(true)
            {}

        bool IsDefaultNewGeometryTypeSet() const { return m_defaultNewGeometryTypeSet; }
        DefaultNewGeometryType GetDefaultNewGeometryType() const { BeAssert(m_defaultNewGeometryTypeSet); return m_defaultNewGeometryType; }

        bool IsDefaultLinePlacementStrategyTypeSet() const { return m_defaultLinePlacementStrategyTypeSet; }
        LinePlacementStrategyType GetDefaultLinePlacementStrategyType() const { BeAssert(m_defaultLinePlacementStrategyTypeSet); return m_defaultLinePlacementStrategyType; }

        bool IsDefaultArcPlacementStrategyTypeSet() const { return m_defaultArcPlacementMethodSet; }
        ArcPlacementMethod GetDefaultArcPlacementStrategyType() const { BeAssert(m_defaultArcPlacementMethodSet); return m_defaultArcPlacementMethod; }

        bool IsDefaultLineStringPlacementStrategyTypeSet() const { return m_defaultLineStringPlacementStrategyTypeSet; }
        LineStringPlacementStrategyType GetDefaultLineStringPlacementStrategyType() const { BeAssert(m_defaultLineStringPlacementStrategyTypeSet); return m_defaultLineStringPlacementStrategyType; }
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ExtrusionManipulationStrategy : public SolidPrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(SolidPrimitiveManipulationStrategy)

    private:
        CurveVectorManipulationStrategyPtr m_baseShapeManipulationStrategy;
        bool m_baseComplete;

        double m_height;
        double m_dynamicHeight;
        double m_fixedHeight;
        bool m_heightSet;
        bool m_dynamicHeightSet;
        bool m_useFixedHeight;

        DVec3d m_sweepDirection;
        DVec3d m_dynamicSweepDirection;
        DVec3d m_fixedSweepDirection;
        bool m_sweepDirectionSet;
        bool m_dynamicSweepDirectionSet;
        bool m_useFixedSweepDirection;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ExtrusionManipulationStrategy();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ExtrusionManipulationStrategy(CurveVectorManipulationStrategyR baseShapeManipulationStrategy);

        double CalculateHeight(DPoint3dCR keyPoint) const;
        DVec3d CalculateSweepDirection(DPoint3dCR keyPoint) const;

        double GetHeight() const;
        DVec3d GetSweepDirection() const;

        DPlane3d GetBasePlane() const;
        DPlane3d GetTopPlane() const;
        DVec3d GetScaledSweepDirection(DPlane3d const& base, DPlane3d const& top, DVec3d sweepDirection) const;

        bool IsSweepDirectionSet() const;

        void SetFixedHeight(double const& newHeight);
        void SetFixedSweepDirection(DVec3d const& newSweepDirection);

    protected:
        virtual GeometryManipulationStrategyCR _GetBaseShapeManipulationStrategy() const { return *m_baseShapeManipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetBaseShapeManipulationStrategyForEdit() { return *m_baseShapeManipulationStrategy; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsBaseComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetBaseComplete(bool value) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> _GetKeyPoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsDynamicKeyPointSet() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ISolidPrimitivePtr _FinishSolidPrimitive() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, bool const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, GeometryManipulationStrategyProperty const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, int const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, double const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, DVec3d const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, DPlane3d const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, RotMatrix const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, Utf8String const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, bvector<double> const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, bvector<Utf8String> const& value) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, double& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, bool& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, DVec3d& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, DPlane3d& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, int& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, RotMatrix& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, Utf8String& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, bvector<double>& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, bvector<Utf8String>& value) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, GeometryManipulationStrategyProperty& value) const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;

        // IRessetableDynamic
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetDynamicState(DynamicStateBaseCR state) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DynamicStateBaseCPtr _GetDynamicState() const override;

    public:
        static constexpr Utf8CP prop_BaseShapeStrategy() { return "BaseShapeStrategy"; }
        static constexpr Utf8CP prop_ContinuousBaseShapePrimitiveComplete() { return "ContinuousBaseShapePrimitiveComplete"; }
        static constexpr Utf8CP prop_Height() { return "Height"; }
        static constexpr Utf8CP prop_FixedHeight() { return "FixedHeight"; }
        static constexpr Utf8CP prop_IsHeightSet() { return "IsHeightSet"; }
        static constexpr Utf8CP prop_IsDynamicHeightSet() { return "IsDynamicHeightSet"; }
        static constexpr Utf8CP prop_UseFixedHeight() { return "UseFixedHeight"; }
        static constexpr Utf8CP prop_IsSweepDirectionSet() { return "IsSweepDirectionSet"; }
        static constexpr Utf8CP prop_IsDynamicSweepDirectionSet() { return "IsDynamicSweepDirectionSet"; }
        static constexpr Utf8CP prop_UseFixedSweepDirection() { return "UseFixedSweepDirection"; }
        static constexpr Utf8CP prop_SweepDirection() { return "SweepDirection"; }
        static constexpr Utf8CP prop_FixedSweepDirection() { return "FixedSweepDirection"; }

        static ExtrusionManipulationStrategyPtr Create() { return new ExtrusionManipulationStrategy(); }
        static ExtrusionManipulationStrategyPtr Create(CurveVectorManipulationStrategyR baseShapeManipulationStrategy) { return new ExtrusionManipulationStrategy(baseShapeManipulationStrategy); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsHeightSet() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ISolidPrimitivePtr FinishExtrusion(bool closedBaseShape = false, bool capped = true) const;
    };

END_BUILDING_SHARED_NAMESPACE