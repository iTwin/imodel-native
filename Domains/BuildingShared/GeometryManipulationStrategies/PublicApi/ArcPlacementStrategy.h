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
    private:
        ArcManipulationStrategyR m_manipulationStrategy;

    protected:
        IArcPlacementMethod(ArcManipulationStrategyR manipulationStrategy)
            : m_manipulationStrategy(manipulationStrategy)
            {}

        ArcManipulationStrategyR GetArcManipulationStrategyForEdit() { return m_manipulationStrategy; }
        ArcManipulationStrategyCR GetArcManipulationStrategy() const { return m_manipulationStrategy; }

        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) = 0;
        virtual void _PopKeyPoint() = 0;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcPlacementStrategy : public CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    private:
        ArcManipulationStrategyPtr m_manipulationStrategy;
        IArcPlacementMethodPtr m_placementMethod;

        static IArcPlacementMethodPtr CreatePlacementMethod(ArcPlacementMethod method, ArcManipulationStrategyR manipulationStrategy);

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcPlacementStrategy(ArcManipulationStrategyR manipulationStrategy, IArcPlacementMethodR method);
    
        DPoint3d CalculateVec90KeyPoint(DPoint3dCR endPoint) const;

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        ArcManipulationStrategyCR GetArcManipulationStrategy() const { return *m_manipulationStrategy; }
        ArcManipulationStrategyR GetArcManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _PopKeyPoint() override;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;

    public:
        static constexpr Utf8CP prop_Normal() { return EllipseManipulationStrategy::prop_Normal(); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static ArcPlacementStrategyPtr Create(ArcPlacementMethod method);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static ArcPlacementStrategyPtr Create(ArcPlacementMethod method, ArcManipulationStrategyR manipulationStrategy);
    };

END_BUILDING_SHARED_NAMESPACE