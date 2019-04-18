/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    
    private:
        Dgn::DgnDbR m_db;

    protected:
        DgnElementManipulationStrategy(Dgn::DgnDbR db)
            : T_Super()
            , m_db(db)
            {}

        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) = 0;
        virtual Dgn::DgnElementPtr _FinishElement() = 0;
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual void _AddWorldOverlay(Dgn::Render::GraphicBuilderR builder, Dgn::ColorDefCR contrastingToBackgroundColor) const;

        DGNELEM_V_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_V_PROPERTY(Dgn::DgnElementId)
        DGNELEM_V_PROPERTY(Dgn::ColorDef)
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

    public:
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement(Dgn::DgnModelR model);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement();

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT void AddWorldOverlay(Dgn::Render::GraphicBuilderR builder, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const;

        DGNELEM_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_PROPERTY(Dgn::DgnElementId)
        DGNELEM_PROPERTY(Dgn::ColorDef)
        using T_Super::SetProperty;
        using T_Super::TryGetProperty;

        Dgn::DgnDbR GetDgnDb() const { return m_db; }
    };

END_BUILDING_SHARED_NAMESPACE