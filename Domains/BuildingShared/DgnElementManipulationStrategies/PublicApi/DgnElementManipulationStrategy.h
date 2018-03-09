/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnElementManipulationStrategies/PublicApi/DgnElementManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define DGNELEM_V_PROPERTY(value_type) \
    virtual void _SetProperty(Utf8CP key, value_type const& value) {} \
    virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const { return BentleyStatus::ERROR; }

#define DGNELEM_PROPERTY(value_type) \
    DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT void SetProperty(Utf8CP key, value_type const& value); \
    DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT BentleyStatus TryGetProperty(Utf8CP key, value_type& value) const;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct DgnElementManipulationStrategy : ElementManipulationStrategy
    {
    DEFINE_T_SUPER(ElementManipulationStrategy)
    
    protected:
        using T_Super::T_Super;

        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) = 0;
        virtual Dgn::DgnElementPtr _FinishElement() = 0;

        DGNELEM_V_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_V_PROPERTY(Dgn::DgnElementId)
        DGNELEM_V_PROPERTY(Dgn::ColorDef)
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

    public:
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement(Dgn::DgnModelR model);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement();

        DGNELEM_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_PROPERTY(Dgn::DgnElementId)
        DGNELEM_PROPERTY(Dgn::ColorDef)
        using T_Super::SetProperty;
        using T_Super::TryGetProperty;
    };

END_BUILDING_SHARED_NAMESPACE