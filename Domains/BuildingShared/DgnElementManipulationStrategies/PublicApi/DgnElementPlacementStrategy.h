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
// @bsiclass                                     Mindaugas.Butkus               05/2018
//=======================================================================================
struct FUSProperty : GeometryManipulationStrategyProperty
    {
    private:
        Formatting::Format m_fus;

    public:
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT FUSProperty();
        FUSProperty& operator=(FUSProperty const&);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT FUSProperty(Formatting::Format const& fus);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Formatting::Format GetFUS() const;
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct DgnElementPlacementStrategy : ElementPlacementStrategy
    {
    DEFINE_T_SUPER(ElementPlacementStrategy)

    private:
        Formatting::Format m_lengthFUS;

        Formatting::Format GetLengthFUS() const;
        Formatting::Format GetAreaFUS() const;
        Formatting::Format GetVolumeFUS() const;

    protected:
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT DgnElementPlacementStrategy(Dgn::DgnDbR db);

        virtual DgnElementManipulationStrategyCR _GetDgnElementManipulationStrategy() const = 0;
        virtual DgnElementManipulationStrategyR _GetDgnElementManipulationStrategyForEdit() = 0;
        virtual ElementManipulationStrategyCR _GetElementManipulationStrategy() const override { return _GetDgnElementManipulationStrategy(); }
        virtual ElementManipulationStrategyR _GetElementManipulationStrategyForEdit() override { return _GetDgnElementManipulationStrategyForEdit(); }

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual Dgn::DgnElementPtr _FinishElement();

        virtual void _AddViewOverlay(Dgn::Render::GraphicBuilderR builder, DRange3dCR viewRange, TransformCR worldToView, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const = 0;
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual void _AddWorldOverlay(Dgn::Render::GraphicBuilderR builder, Dgn::ColorDefCR contrastingToBackgroundColor) const;

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, GeometryManipulationStrategyProperty const& value) override;
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, GeometryManipulationStrategyProperty& value) const override;

        DGNELEM_P_V_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_P_V_PROPERTY(Dgn::DgnElementId)
        DGNELEM_P_V_PROPERTY(Dgn::ColorDef)
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Utf8String GetFormattedLength(double length) const;
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Utf8String GetFormattedArea(double area) const;
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Utf8String GetFormattedVolume(double volume) const;

    public:
        static constexpr Utf8CP prop_LengthFUS() { return "LengthFUS"; } // Set/Get
        static constexpr Utf8CP prop_AreaFUS() { return "AreaFUS"; }     // Get
        static constexpr Utf8CP prop_VolumeFUS() { return "VolumeFUS"; } // Get

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement(Dgn::DgnModelR model);
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement();

        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT void AddViewOverlay(Dgn::Render::GraphicBuilderR builder, DRange3dCR viewRange, TransformCR worldToView, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const;
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT void AddWorldOverlay(Dgn::Render::GraphicBuilderR builder, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const;

        DGNELEM_P_PROPERTY(Dgn::DgnElementCP)
        DGNELEM_P_PROPERTY(Dgn::DgnElementId)
        DGNELEM_P_PROPERTY(Dgn::ColorDef)
        using T_Super::SetProperty;
        using T_Super::TryGetProperty;

        //! DO NOT CALL THIS METHOD IN A CONSTRUCTOR!!!
        //! This method's implementation uses a pure virtual method to retrieve
        //! the DgnDb. That virtual method is used to retrieve a member
        //! stored in a descendant class. That member will be initialized only after
        //! DgnElementPlacementStrategy is initialized.
        //! Calling this method in a constructor will cause a crash.
        DGNELEMENTMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnDbR GetDgnDb() const;
    };

END_BUILDING_SHARED_NAMESPACE