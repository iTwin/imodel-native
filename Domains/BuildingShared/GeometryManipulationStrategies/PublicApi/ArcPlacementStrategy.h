/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class ArcPlacementMethod
    {
    StartCenter = 0,
    CenterStart = 1,
    StartMidEnd = 2,
    StartEndMid = 3
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct IArcPlacementMethod : IRefCounted
    {
    protected:
        virtual ~IArcPlacementMethod() {}

    protected:
        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) = 0;
        virtual void _PopKeyPoint() = 0;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) = 0;
        virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) = 0;
        virtual ArcPlacementMethod _GetMethod() const = 0;
        virtual bvector<DPoint3d> _GetKeyPoints() const = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcPlacementMethod GetMethod() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bvector<DPoint3d> GetKeyPoints() const;
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct IArcPlacementStrategy
    {
    protected:
        virtual ~IArcPlacementStrategy() {}

        virtual void _SetPlacementMethod(ArcPlacementMethod method) = 0;

        virtual void _SetUseSweep(bool useSweep) = 0;
        virtual void _SetSweep(double sweep) = 0;

        virtual void _SetUseRadius(bool useRadius) = 0;
        virtual void _SetRadius(double radius) = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetPlacementMethod(ArcPlacementMethod method);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetUseSweep(bool useSweep);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetSweep(double sweep);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetUseRadius(bool useRadius);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetRadius(double radius);
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               11/2018
//=======================================================================================
struct IArcKeyPointContainer
    {
    protected:
        virtual ~IArcKeyPointContainer() {}

        virtual bool _TryGetStartKeyPoint(DPoint3dR) const = 0;
        virtual bool _TryGetCenterKeyPoint(DPoint3dR) const = 0;
        virtual bool _TryGetMidKeyPoint(DPoint3dR) const = 0;
        virtual bool _TryGetEndKeyPoint(DPoint3dR) const = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool TryGetStartKeyPoint(DPoint3dR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool TryGetCenterKeyPoint(DPoint3dR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool TryGetMidKeyPoint(DPoint3dR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool TryGetEndKeyPoint(DPoint3dR) const;
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcPlacementStrategy 
    : public CurvePrimitivePlacementStrategy
    , IArcPlacementStrategy
    , IArcKeyPointContainer
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    private:
        ArcManipulationStrategyPtr m_manipulationStrategy;
        IArcPlacementMethodPtr m_placementMethod;

        static IArcPlacementMethodPtr CreatePlacementMethod(ArcPlacementMethod method, ArcManipulationStrategyR manipulationStrategy);

        template<bool(ArcManipulationStrategy::*isSet)() const, DPoint3d(ArcManipulationStrategy::*get)() const> bool TryGetKeyPoint(ArcManipulationStrategyCP strategy, DPoint3dR pointToSet) const;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcPlacementStrategy(ArcManipulationStrategyR manipulationStrategy, IArcPlacementMethodR method);
    
        DPoint3d CalculateVec90KeyPoint(DPoint3dCR endPoint) const;

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        ArcManipulationStrategyCR GetArcManipulationStrategy() const { return *m_manipulationStrategy; }
        ArcManipulationStrategyR GetArcManipulationStrategyForEdit() { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _PopKeyPoint() override;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;

        virtual bvector<DPoint3d> _GetKeyPoints() const override;

        // IArcPlacementStrategy
        virtual void _SetPlacementMethod(ArcPlacementMethod method) override;
        virtual void _SetUseSweep(bool useSweep) override;
        virtual void _SetSweep(double sweep) override;
        virtual void _SetUseRadius(bool useRadius) override;
        virtual void _SetRadius(double radius) override;

        // IArcKeyPointContainer
        virtual bool _TryGetStartKeyPoint(DPoint3dR) const override;
        virtual bool _TryGetCenterKeyPoint(DPoint3dR) const override;
        virtual bool _TryGetMidKeyPoint(DPoint3dR) const override;
        virtual bool _TryGetEndKeyPoint(DPoint3dR) const override;

    public:
        static constexpr Utf8CP prop_Normal() { return EllipseManipulationStrategy::prop_Normal(); }
        static constexpr Utf8CP prop_WorkingPlane() { return EllipseManipulationStrategy::prop_WorkingPlane(); }

        static constexpr Utf8CP prop_UseSweep() { return ArcManipulationStrategy::prop_UseSweep(); }
        static constexpr Utf8CP prop_Sweep() { return ArcManipulationStrategy::prop_Sweep(); }
        static constexpr Utf8CP prop_UseRadius() { return ArcManipulationStrategy::prop_UseRadius(); }
        static constexpr Utf8CP prop_Radius() { return ArcManipulationStrategy::prop_Radius(); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcPlacementMethod GetPlacementMethod() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static ArcPlacementStrategyPtr Create(ArcPlacementMethod method);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static ArcPlacementStrategyPtr Create(ArcPlacementMethod method, ArcManipulationStrategyR manipulationStrategy);
    };

END_BUILDING_SHARED_NAMESPACE