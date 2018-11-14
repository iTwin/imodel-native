/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define KEY_POINT_ACCESSOR_DECL(name, index) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT DPoint3d Get##name() const; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT size_t Get##name##Index() const; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool Is##name##Set() const; \
    void Set##name(DPoint3dCR newValue); \
    void Reset##name(); \
    void SetDynamic##name(DPoint3dCR newValue); \
    bool Is##name##Dynamic() const;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ArcManipulationStrategy : public EllipseManipulationStrategy
    {
    DEFINE_T_SUPER(EllipseManipulationStrategy)

    private:
        static const size_t s_startIndex = 0;
        static const size_t s_midPointIndex = 1;
        static const size_t s_centerIndex = 2;
        static const size_t s_endIndex = 3;

        bool m_useSweep;
        double m_sweep;

        bool m_useRadius;
        double m_radius;

        ICurvePrimitivePtr m_lastArc;

        static bool PointsOnLine(DPoint3dCR, DPoint3dCR, DPoint3dCR);
        void UpdateLastArc();

        //! Returns the Start keypoint rotated around the Center keypoint so that
        //! the vector that goes from center to this point is perpendicular to the normal.
        DPoint3d GetArcStart() const;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcManipulationStrategy();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<IGeometryPtr> _FinishConstructionGeometry() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ArcPlacementStrategyPtr _CreateArcPlacementStrategy(ArcPlacementMethod method) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _OnKeyPointsChanged() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsEmpty() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsSingleKeyPointLeft() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DPoint3d _GetLastKeyPoint() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual DPoint3d _GetFirstKeyPoint() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _Clear() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, bool const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, bool& value) const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, int const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, double const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, double& value) const override;

        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

    public:
        static constexpr Utf8CP prop_UseSweep() { return "UseSweep"; }
        static constexpr Utf8CP prop_Sweep() { return "Sweep"; }
        static constexpr Utf8CP prop_UseRadius() { return "UseRadius"; }
        static constexpr Utf8CP prop_Radius() { return "Radius"; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static ArcManipulationStrategyPtr Create();
        static ArcManipulationStrategyPtr Create(ICurvePrimitiveCR arc);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void Init(ICurvePrimitiveCR arc);

        KEY_POINT_ACCESSOR_DECL(Start, s_startIndex)
        KEY_POINT_ACCESSOR_DECL(Center, s_centerIndex)
        KEY_POINT_ACCESSOR_DECL(Mid, s_midPointIndex)
        KEY_POINT_ACCESSOR_DECL(End, s_endIndex)
    };

END_BUILDING_SHARED_NAMESPACE