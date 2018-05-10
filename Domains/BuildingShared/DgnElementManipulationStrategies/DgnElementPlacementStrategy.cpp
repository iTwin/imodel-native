/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnElementManipulationStrategies/DgnElementPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/DgnElementManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
FUSProperty::FUSProperty
(
    Formatting::FormatUnitSet const& fus
)
    : m_fus(fus)
    {
    }

FUSProperty::FUSProperty
(
)
    : m_fus(Formatting::FormatUnitSet("M(Meters4u)"))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Formatting::FormatUnitSet FUSProperty::GetFUS() const
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
DgnElementPlacementStrategy::DgnElementPlacementStrategy()
    : T_Super()
    , m_lengthFUS(Formatting::FormatUnitSet("M(Meters4u)"))
    {}

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
Formatting::FormatUnitSet DgnElementPlacementStrategy::GetLengthFUS() const
    {
    return m_lengthFUS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Formatting::FormatUnitSet DgnElementPlacementStrategy::GetAreaFUS() const
    {
    Utf8String unitName = m_lengthFUS.GetUnitName();
    Utf8String format = "real4u";
    Utf8PrintfString formatDescription("SQ.%s(%s)", unitName, format);

    return Formatting::FormatUnitSet(formatDescription.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Formatting::FormatUnitSet DgnElementPlacementStrategy::GetVolumeFUS() const
    {
    Utf8String unitName = m_lengthFUS.GetUnitName();
    Utf8String format = "real4u";
    Utf8PrintfString formatDescription("CUB.%s(%s)", unitName, format);

    return Formatting::FormatUnitSet(formatDescription.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String DgnElementPlacementStrategy::GetFormattedLength
(
    double length
) const
    {
    Units::UnitCP meterUnit = Units::UnitRegistry::Instance().LookupUnit("M");
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
    Units::UnitCP sqMeterUnit = Units::UnitRegistry::Instance().LookupUnit("SQ.M");
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
    Units::UnitCP cubMeterUnit = Units::UnitRegistry::Instance().LookupUnit("CUB.M");
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