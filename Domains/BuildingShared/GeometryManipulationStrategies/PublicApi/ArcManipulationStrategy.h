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
    DPoint3d Get##name() const; \
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

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcManipulationStrategy();

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;

        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;
        virtual ArcPlacementStrategyPtr _CreateArcPlacementStrategy(ArcPlacementMethod method) override;


        virtual void _OnKeyPointsChanged() override;

        virtual bool _IsComplete() const override;

        virtual bool _IsEmpty() const override;
        virtual bool _IsSingleKeyPointLeft() const override;
        virtual DPoint3d _GetLastKeyPoint() const override;
        virtual DPoint3d _GetFirstKeyPoint() const override;
        virtual void _Clear() override;

        CurvePrimitiveManipulationStrategyPtr _Clone() const override;

        virtual void _SetProperty(Utf8CP key, bool const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, bool& value) const override;

        virtual void _SetProperty(Utf8CP key, int const& value) override;
        virtual void _SetProperty(Utf8CP key, double const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double& value) const override;

        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

    public:
        static constexpr Utf8CP prop_UseSweep() { return "UseSweep"; }
        static constexpr Utf8CP prop_Sweep() { return "Sweep"; }
        static constexpr Utf8CP prop_UseRadius() { return "UseRadius"; }
        static constexpr Utf8CP prop_Radius() { return "Radius"; }

        static ArcManipulationStrategyPtr Create() { return new ArcManipulationStrategy(); }
        static ArcManipulationStrategyPtr Create(ICurvePrimitiveCR arc);

        KEY_POINT_ACCESSOR_DECL(Start, s_startIndex)
        KEY_POINT_ACCESSOR_DECL(Center, s_centerIndex)
        KEY_POINT_ACCESSOR_DECL(Mid, s_midPointIndex)
        KEY_POINT_ACCESSOR_DECL(End, s_endIndex)
    };

END_BUILDING_SHARED_NAMESPACE