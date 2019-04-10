/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnElementManipulationStrategies/DgnElementPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/DgnElementManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

#define Standard_Units_Schema "Units"
#define Standard_Formats_Schema "Formats"
#define Squared_Template "SQ_%s"
#define Cubed_Template "CUB_%s"

#define DefaultRealU "DefaultRealU" // real4u

#define Meter "M"
#define Meter_Squared "SQ_M"
#define Meter_Cubed "CUB_M"

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
FUSProperty::FUSProperty
(
    Formatting::Format const& fus
)
    : m_fus(fus)
    {
    }

FUSProperty::FUSProperty
(
)
    : m_fus(Formatting::Format())
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Formatting::Format FUSProperty::GetFUS() const
    {
    return m_fus;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
FUSProperty& FUSProperty::operator=(FUSProperty const& other)
    {
    m_fus = other.GetFUS();
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
DgnElementPlacementStrategy::DgnElementPlacementStrategy
(
    Dgn::DgnDbR db
)
    : T_Super()
    {
    ECN::ECUnitCP unit = db.Schemas().GetUnit(Standard_Units_Schema, Meter);
    BeAssert(nullptr != unit);
    ECN::ECFormatCP format = db.Schemas().GetFormat(Standard_Formats_Schema, DefaultRealU);
    BeAssert(nullptr != format);

    // This creates a copy of the original format so that we can make the precision change.
    Formatting::Format formatOverride(*format);
    formatOverride.GetNumericSpecP()->SetPrecision(Formatting::DecimalPrecision::Precision4);

    auto compSpec = formatOverride.GetCompositeSpecP();
    if (nullptr == compSpec)
       {
       Formatting::CompositeValueSpec comp;
       Formatting::CompositeValueSpec::CreateCompositeSpec(comp, bvector<BEU::UnitCP>{unit});
       formatOverride.SetCompositeSpec(comp);
       }

    m_lengthFUS = formatOverride;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::_FinishElement
(
    Dgn::DgnModelR model
)
    {
    return _GetDgnElementManipulationStrategyForEdit().FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::_FinishElement()
    {
    return _GetDgnElementManipulationStrategyForEdit().FinishElement();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void DgnElementPlacementStrategy::AddWorldOverlay
(
    Dgn::Render::GraphicBuilderR builder, 
    Dgn::ColorDefCR contrastingToBackgroundColor
) const
    {
    _AddWorldOverlay(builder, contrastingToBackgroundColor);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void DgnElementPlacementStrategy::_AddWorldOverlay
(
    Dgn::Render::GraphicBuilderR builder,
    Dgn::ColorDefCR contrastingToBackgroundColor
) const
    {
    _GetDgnElementManipulationStrategy().AddWorldOverlay(builder, contrastingToBackgroundColor);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::FinishElement
(
    Dgn::DgnModelR model
)
    {
    return _FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementPlacementStrategy::FinishElement()
    {
    return _FinishElement();
    }

#define EPS_V_PROPERTY_IMPL(value_type)                                                                 \
    void DgnElementPlacementStrategy::_SetProperty(Utf8CP key, value_type const& value)                 \
        {                                                                                               \
        _GetDgnElementManipulationStrategyForEdit().SetProperty(key, value);                            \
        }                                                                                               \
    BentleyStatus DgnElementPlacementStrategy::_TryGetProperty(Utf8CP key, value_type& value) const     \
        {                                                                                               \
        return _GetDgnElementManipulationStrategy().TryGetProperty(key, value);                         \
        }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             03/2018
//---------------+---------------+---------------+---------------+---------------+------
void DgnElementPlacementStrategy::AddViewOverlay
(
    Dgn::Render::GraphicBuilderR builder,
    DRange3dCR viewRange,
    TransformCR worldToView,
    Dgn::ColorDefCR contrastingToBackgroundColor
) const
    {
    _AddViewOverlay(builder, viewRange, worldToView, contrastingToBackgroundColor);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbR DgnElementPlacementStrategy::GetDgnDb() const
    {
    return _GetDgnElementManipulationStrategy().GetDgnDb();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Formatting::Format DgnElementPlacementStrategy::GetLengthFUS() const
    {
    return m_lengthFUS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Formatting::Format DgnElementPlacementStrategy::GetAreaFUS() const
     {
    Utf8String unitName = m_lengthFUS.GetCompositeMajorUnit()->GetName();

    Utf8PrintfString squaredUnitName(Squared_Template, unitName.c_str());
    ECN::ECUnitCP unit = GetDgnDb().Schemas().GetUnit(Standard_Units_Schema, squaredUnitName.c_str());

    ECN::ECFormatCP format = GetDgnDb().Schemas().GetFormat(Standard_Formats_Schema, DefaultRealU);

    // This creates a copy of the original format so that we can make the precision change.
    Formatting::Format formatOverride(*format);
    formatOverride.GetNumericSpecP()->SetPrecision(Formatting::DecimalPrecision::Precision4);

    auto compSpec = formatOverride.GetCompositeSpecP();
    if (nullptr == compSpec)
        {
        Formatting::CompositeValueSpec comp;
        Formatting::CompositeValueSpec::CreateCompositeSpec(comp, bvector<BEU::UnitCP>{unit});
        formatOverride.SetCompositeSpec(comp);
        }

    return formatOverride;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Formatting::Format DgnElementPlacementStrategy::GetVolumeFUS() const
    {
    Utf8String unitName = m_lengthFUS.GetCompositeMajorUnit()->GetName();

    Utf8PrintfString cubedUnitName(Cubed_Template, unitName.c_str());
    ECN::ECUnitCP unit = GetDgnDb().Schemas().GetUnit(Standard_Units_Schema, cubedUnitName.c_str());

    ECN::ECFormatCP format = GetDgnDb().Schemas().GetFormat(Standard_Formats_Schema, DefaultRealU);
 
    // This creates a copy of the original format so that we can make the precision change.
    Formatting::Format formatOverride(*format);
    formatOverride.GetNumericSpecP()->SetPrecision(Formatting::DecimalPrecision::Precision4);

    auto compSpec = formatOverride.GetCompositeSpecP();
    if (nullptr == compSpec)
        {
        Formatting::CompositeValueSpec comp;
        Formatting::CompositeValueSpec::CreateCompositeSpec(comp, bvector<BEU::UnitCP>{unit});
        formatOverride.SetCompositeSpec(comp);
        }

    return formatOverride;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String DgnElementPlacementStrategy::GetFormattedLength
(
    double length
) const
    {
    Units::UnitCP meterUnit = GetDgnDb().Schemas().GetUnit(Standard_Units_Schema, Meter);
    BeAssert(nullptr != meterUnit);
    Units::Quantity quantity(length, *meterUnit);

    return GetLengthFUS().FormatQuantity(quantity, "");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String DgnElementPlacementStrategy::GetFormattedArea
(
    double area
) const
    {
    Units::UnitCP sqMeterUnit = GetDgnDb().Schemas().GetUnit(Standard_Units_Schema, Meter_Squared);
    BeAssert(nullptr != sqMeterUnit);
    Units::Quantity quantity(area, *sqMeterUnit);
    
    return GetAreaFUS().FormatQuantity(quantity, "");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String DgnElementPlacementStrategy::GetFormattedVolume
(
    double volume
) const
    {
    Units::UnitCP cubMeterUnit = GetDgnDb().Schemas().GetUnit(Standard_Units_Schema, Meter_Cubed);
    BeAssert(nullptr != cubMeterUnit);
    Units::Quantity quantity(volume, *cubMeterUnit);

    return GetVolumeFUS().FormatQuantity(quantity, "");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void DgnElementPlacementStrategy::_SetProperty
(
    Utf8CP key, 
    GeometryManipulationStrategyProperty const& value
)
    {
    if (0 == strcmp(key, prop_LengthFUS()))
        {
        FUSProperty const* fusProp = dynamic_cast<FUSProperty const*>(&value);
        if (nullptr != fusProp)
            {
            m_lengthFUS = fusProp->GetFUS();
            }
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DgnElementPlacementStrategy::_TryGetProperty
(
    Utf8CP key, 
    GeometryManipulationStrategyProperty& value
) const
    {
    FUSProperty* fusProp = dynamic_cast<FUSProperty*>(&value);
    if (nullptr != fusProp)
        {
        if (0 == strcmp(key, prop_LengthFUS()))
            {
            *fusProp = FUSProperty(GetLengthFUS());
            return BentleyStatus::SUCCESS;
            }
        if (0 == strcmp(key, prop_AreaFUS()))
            {
            *fusProp = FUSProperty(GetAreaFUS());
            return BentleyStatus::SUCCESS;
            }
        if (0 == strcmp(key, prop_VolumeFUS()))
            {
            *fusProp = FUSProperty(GetVolumeFUS());
            return BentleyStatus::SUCCESS;
            }
        }

    return T_Super::_TryGetProperty(key, value);
    }

EPS_V_PROPERTY_IMPL(Dgn::DgnElementCP)
EPS_V_PROPERTY_IMPL(Dgn::DgnElementId)
EPS_V_PROPERTY_IMPL(Dgn::ColorDef)

#define SET_PROPERTY_IMPL(value_type) \
    void DgnElementPlacementStrategy::SetProperty(Utf8CP key, value_type const& value) \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return; } \
        _SetProperty(key, value); \
        _OnPropertySet(key); \
        }
#define TRY_GET_PROPERTY_IMPL(value_type) \
    BentleyStatus DgnElementPlacementStrategy::TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return BentleyStatus::ERROR; } \
        return _TryGetProperty(key, value); \
        }
#define SET_TRYGET_PROPERTY_IMPL(value_type) \
    SET_PROPERTY_IMPL(value_type) \
    TRY_GET_PROPERTY_IMPL(value_type)

SET_TRYGET_PROPERTY_IMPL(Dgn::DgnElementCP)
SET_TRYGET_PROPERTY_IMPL(Dgn::DgnElementId)
SET_TRYGET_PROPERTY_IMPL(Dgn::ColorDef)