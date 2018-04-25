/*--------------------------------------------------------------------------------------+
|
|     $Source: StrategyPlatform/PublicAPI/GeometricElementManipulator.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct GeometricElementManipulator : Dgn::PointDragManipulator
    {
    DEFINE_T_SUPER(Dgn::PointDragManipulator)

        struct ControlPoint : Dgn::PointDragManipulator::ControlPoint
        {
        DEFINE_T_SUPER(Dgn::PointDragManipulator::ControlPoint)

        private:
            DgnElementManipulationStrategyR m_strategy;
            size_t m_keyPointIndex;

        public:
            ControlPoint(DgnElementManipulationStrategyR strategy, size_t keyPointIndex, DPoint3dCR point)
                : T_Super(point)
                , m_keyPointIndex(keyPointIndex)
                , m_strategy(strategy)
                {}

            void UpdatePoint(DPoint3dCR newPoint, bool isDynamics);
        };

    protected:
        GeometricElementManipulator() {}

        virtual DgnElementManipulationStrategyR _GetStrategy() = 0;
        virtual Dgn::GeometricElementR _GetElement() = 0;

        STRATEGYPLATFORM_EXPORT virtual bool _IsDisplayedInView(Dgn::DgnViewportR vp) override;
        STRATEGYPLATFORM_EXPORT virtual StatusInt _DoModify(Dgn::DgnButtonEventCR ev, bool isDynamics) override;
        STRATEGYPLATFORM_EXPORT virtual bool _DoCreateControls() override;
        STRATEGYPLATFORM_EXPORT virtual StatusInt _OnModifyAccept(Dgn::DgnButtonEventCR ev) override;
        STRATEGYPLATFORM_EXPORT virtual void _OnModifyCancel(Dgn::DgnButtonEventCR ev) override;
    };

END_BUILDING_SHARED_NAMESPACE