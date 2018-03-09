/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnElementManipulationStrategies/PublicApi/DgnElementPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define DGNELEM_P_V_PROPERTY(value_type) \
    DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, value_type const& value); \
    DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const;

#define DGNELEM_P_PROPERTY(value_type) \
    DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT void SetProperty(Utf8CP key, value_type const& value); \
    DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT BentleyStatus TryGetProperty(Utf8CP key, value_type& value) const;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct DgnElementPlacementStrategy : ElementPlacementStrategy
    {
    DEFINE_T_SUPER(ElementPlacementStrategy)

    protected:
        using T_Super::T_Super;

        virtual DgnElementManipulationStrategyCR _GetDgnElementManipulationStrategy() const = 0;
        virtual DgnElementManipulationStrategyR _GetDgnElementManipulationStrategyForEdit() = 0;
        virtual ElementManipulationStrategyCR _GetElementManipulationStrategy() const override { return _GetDgnElementManipulationStrategy(); }
        virtual ElementManipulationStrategyR _GetElementManipulationStrategyForEdit() override { return _GetDgnElementManipulationStrategyForEdit(); }

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual Dgn::DgnElementPtr _FinishElement();

        virtual void _AddViewOverlay(Dgn::Render::GraphicBuilderR builder, DRange3dCR viewRange, TransformCR worldToView, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const = 0;

        DGNELEM_P_V_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_P_V_PROPERTY(Dgn::DgnElementId)
        DGNELEM_P_V_PROPERTY(Dgn::ColorDef)
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

    public:
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement(Dgn::DgnModelR model);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement();

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT void AddViewOverlay(Dgn::Render::GraphicBuilderR builder, DRange3dCR viewRange, TransformCR worldToView, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const;

        DGNELEM_P_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_P_PROPERTY(Dgn::DgnElementId)
        DGNELEM_P_PROPERTY(Dgn::ColorDef)
        using T_Super::SetProperty;
        using T_Super::TryGetProperty;
    };

END_BUILDING_SHARED_NAMESPACE