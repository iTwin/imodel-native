/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/AnnotationTextStyle.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGN
using namespace flatbuffers;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationTextStylePropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((AnnotationTextStyleProperty)key)
        {
        case AnnotationTextStyleProperty::ColorType:
        case AnnotationTextStyleProperty::ColorValue:
        case AnnotationTextStyleProperty::FontId:
        case AnnotationTextStyleProperty::IsBold:
        case AnnotationTextStyleProperty::IsItalic:
        case AnnotationTextStyleProperty::IsUnderlined:
        case AnnotationTextStyleProperty::StackedFractionType:
            return true;

        default:
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationTextStylePropertyBag::_IsRealProperty(T_Key key) const
    {
    switch ((AnnotationTextStyleProperty)key)
        {
        case AnnotationTextStyleProperty::Height:
        case AnnotationTextStyleProperty::LineSpacingFactor:
        case AnnotationTextStyleProperty::StackedFractionScale:
        case AnnotationTextStyleProperty::SubScriptOffsetFactor:
        case AnnotationTextStyleProperty::SubScriptScale:
        case AnnotationTextStyleProperty::SuperScriptOffsetFactor:
        case AnnotationTextStyleProperty::SuperScriptScale:
        case AnnotationTextStyleProperty::WidthFactor:
            return true;

        default:
            return false;
        }
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

#define PROP_Data "Data"
#define PROP_Description "Descr"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(AnnotationTextStyleHandler);

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void AnnotationTextStyleHandler::_TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) // *** WIP_AUTO_HANDLED_PROPERTIES
    {
    T_Super::_TEMPORARY_GetPropertyHandlingCustomAttributes(params);
    params.Add(PROP_Data);
    params.Add(PROP_Description);
    }
}
END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTextStyle::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& select, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(select, params);
    if (DgnDbStatus::Success != status)
        return status;

    int dataSize = 0;
    ByteCP data = static_cast<ByteCP>(select.GetValueBinary(params.GetSelectIndex(PROP_Data), &dataSize));
    if (SUCCESS != AnnotationTextStylePersistence::DecodeFromFlatBuf(*this, data, static_cast<size_t>(dataSize)))
        return DgnDbStatus::BadArg;

    m_description.AssignOrClear(select.GetValueText(params.GetSelectIndex(PROP_Description)));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
static DgnDbStatus bindParams(BeSQLite::EC::ECSqlStatement& stmt, AnnotationTextStyleCR style)
    {
    bvector<Byte> data;
    if (SUCCESS != AnnotationTextStylePersistence::EncodeAsFlatBuf(data, style, AnnotationTextStylePersistence::FlatBufEncodeOptions::Default))
        return DgnDbStatus::BadArg;

    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROP_Description), style.GetDescription().c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;

    if (ECSqlStatus::Success != stmt.BindBinary(stmt.GetParameterIndex(PROP_Data), &data[0], static_cast<int>(data.size()), IECSqlBinder::MakeCopy::Yes))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTextStyle::_BindInsertParams(BeSQLite::EC::ECSqlStatement& insert)
    {
    DgnDbStatus status = T_Super::_BindInsertParams(insert);
    if (DgnDbStatus::Success != status)
        return status;

    return bindParams(insert, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTextStyle::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& update)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(update);
    if (DgnDbStatus::Success != status)
        return status;

    return bindParams(update, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void AnnotationTextStyle::_CopyFrom(DgnElementCR src)
    {
    T_Super::_CopyFrom(src);
    
    AnnotationTextStyleCP rhs = dynamic_cast<AnnotationTextStyleCP>(&src);
    if (nullptr == rhs)
        return;
    
    m_description = rhs->m_description;
    m_data = rhs->m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
void AnnotationTextStyle::_RemapIds(DgnImportContext& context)
    {
    DgnFontId srcFontId = GetFontId();
    DgnFontId dstFontId = context.RemapFont(srcFontId);
    SetFontId(dstFontId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static AnnotationTextStylePropertyBag::T_Integer getIntegerValue(AnnotationTextStylePropertyBagCR data, AnnotationTextStyleProperty key, AnnotationTextStylePropertyBag::T_Integer defaultValue)
    {
    if (data.HasProperty(key))
        return data.GetIntegerProperty(key);

    return defaultValue;
    }
static void setIntegerValue(AnnotationTextStylePropertyBagR data, AnnotationTextStyleProperty key, AnnotationTextStylePropertyBag::T_Integer defaultValue, AnnotationTextStylePropertyBag::T_Integer value)
    {
    if (value == defaultValue)
        data.ClearProperty(key);
    else
        data.SetIntegerProperty(key, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static AnnotationTextStylePropertyBag::T_Real getRealValue(AnnotationTextStylePropertyBagCR data, AnnotationTextStyleProperty key, AnnotationTextStylePropertyBag::T_Real defaultValue)
    {
    if (data.HasProperty(key))
        return data.GetRealProperty(key);

    return defaultValue;
    }
static void setRealValue(AnnotationTextStylePropertyBagR data, AnnotationTextStyleProperty key, AnnotationTextStylePropertyBag::T_Real defaultValue, AnnotationTextStylePropertyBag::T_Real value)
    {
    if (value == defaultValue)
        data.ClearProperty(key);
    else
        data.SetRealProperty(key, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_COLOR_TYPE_VALUE = (AnnotationTextStylePropertyBag::T_Integer)AnnotationColorType::ByCategory;
AnnotationColorType AnnotationTextStyle::GetColorType() const { return (AnnotationColorType)getIntegerValue(m_data, AnnotationTextStyleProperty::ColorType, DEFAULT_COLOR_TYPE_VALUE); }
void AnnotationTextStyle::SetColorType(AnnotationColorType value) { setIntegerValue(m_data, AnnotationTextStyleProperty::ColorType, DEFAULT_COLOR_TYPE_VALUE, (AnnotationTextStylePropertyBag::T_Integer)value); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_COLOR_VALUE_VALUE = 0;
ColorDef AnnotationTextStyle::GetColorValue() const { return ColorDef((uint32_t)getIntegerValue(m_data, AnnotationTextStyleProperty::ColorValue, DEFAULT_COLOR_VALUE_VALUE)); }
void AnnotationTextStyle::SetColorValue(ColorDef value) { setIntegerValue(m_data, AnnotationTextStyleProperty::ColorValue, DEFAULT_COLOR_VALUE_VALUE, value.GetValue()); }

static const int64_t DEFAULT_FONTID_VALUE = 0; // See definition of BeServerIssuedId, of which DgnFontId is a sub-class.
DgnFontId AnnotationTextStyle::GetFontId() const { return DgnFontId((uint64_t)getIntegerValue(m_data, AnnotationTextStyleProperty::FontId, DEFAULT_FONTID_VALUE)); }
void AnnotationTextStyle::SetFontId(DgnFontId value) { setIntegerValue(m_data, AnnotationTextStyleProperty::FontId, DEFAULT_FONTID_VALUE, value.GetValue()); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_HEIGHT_VALUE = 1.0;
double AnnotationTextStyle::GetHeight() const { return getRealValue(m_data, AnnotationTextStyleProperty::Height, DEFAULT_HEIGHT_VALUE); }
void AnnotationTextStyle::SetHeight(double value) { setRealValue(m_data, AnnotationTextStyleProperty::Height, DEFAULT_HEIGHT_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_LINESPACINGFACTOR_VALUE = 0.5;
double AnnotationTextStyle::GetLineSpacingFactor() const { return getRealValue(m_data, AnnotationTextStyleProperty::LineSpacingFactor, DEFAULT_LINESPACINGFACTOR_VALUE); }
void AnnotationTextStyle::SetLineSpacingFactor(double value) { setRealValue(m_data, AnnotationTextStyleProperty::LineSpacingFactor, DEFAULT_LINESPACINGFACTOR_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_ISBOLD_VALUE = 0;
bool AnnotationTextStyle::IsBold() const { return (0 != getIntegerValue(m_data, AnnotationTextStyleProperty::IsBold, DEFAULT_ISBOLD_VALUE)); }
void AnnotationTextStyle::SetIsBold(bool value) { setIntegerValue(m_data, AnnotationTextStyleProperty::IsBold, DEFAULT_ISBOLD_VALUE, (value ? 1 : 0)); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_ISITALIC_VALUE = 0;
bool AnnotationTextStyle::IsItalic() const { return (0 != getIntegerValue(m_data, AnnotationTextStyleProperty::IsItalic, DEFAULT_ISITALIC_VALUE)); }
void AnnotationTextStyle::SetIsItalic(bool value) { setIntegerValue(m_data, AnnotationTextStyleProperty::IsItalic, DEFAULT_ISITALIC_VALUE, (value ? 1 : 0)); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_ISUNDERLINED_VALUE = 0;
bool AnnotationTextStyle::IsUnderlined() const { return (0 != getIntegerValue(m_data, AnnotationTextStyleProperty::IsUnderlined, DEFAULT_ISUNDERLINED_VALUE)); }
void AnnotationTextStyle::SetIsUnderlined(bool value) { setIntegerValue(m_data, AnnotationTextStyleProperty::IsUnderlined, DEFAULT_ISUNDERLINED_VALUE, (value ? 1 : 0)); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_STACKEDFRACTIONSCALE_VALUE = 0.7;
double AnnotationTextStyle::GetStackedFractionScale() const { return getRealValue(m_data, AnnotationTextStyleProperty::StackedFractionScale, DEFAULT_STACKEDFRACTIONSCALE_VALUE); }
void AnnotationTextStyle::SetStackedFractionScale(double value) { setRealValue(m_data, AnnotationTextStyleProperty::StackedFractionScale, DEFAULT_STACKEDFRACTIONSCALE_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_STACKEDFRACTIONTYPE_VALUE = (AnnotationTextStylePropertyBag::T_Integer)AnnotationStackedFractionType::HorizontalBar;
AnnotationStackedFractionType AnnotationTextStyle::GetStackedFractionType() const { return (AnnotationStackedFractionType)getIntegerValue(m_data, AnnotationTextStyleProperty::StackedFractionType, DEFAULT_STACKEDFRACTIONTYPE_VALUE); }
void AnnotationTextStyle::SetStackedFractionType(AnnotationStackedFractionType value) { setIntegerValue(m_data, AnnotationTextStyleProperty::StackedFractionType, DEFAULT_STACKEDFRACTIONTYPE_VALUE, (uint32_t)value); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_SUBSCRIPTOFFSETFACTOR_VALUE = -0.15;
double AnnotationTextStyle::GetSubScriptOffsetFactor() const { return getRealValue(m_data, AnnotationTextStyleProperty::SubScriptOffsetFactor, DEFAULT_SUBSCRIPTOFFSETFACTOR_VALUE); }
void AnnotationTextStyle::SetSubScriptOffsetFactor(double value) { setRealValue(m_data, AnnotationTextStyleProperty::SubScriptOffsetFactor, DEFAULT_SUBSCRIPTOFFSETFACTOR_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_SUBSCRIPTSCALE_VALUE = (2.0 / 3.0);
double AnnotationTextStyle::GetSubScriptScale() const { return getRealValue(m_data, AnnotationTextStyleProperty::SubScriptScale, DEFAULT_SUBSCRIPTSCALE_VALUE); }
void AnnotationTextStyle::SetSubScriptScale(double value) { setRealValue(m_data, AnnotationTextStyleProperty::SubScriptScale, DEFAULT_SUBSCRIPTSCALE_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_SUPERSCRIPTOFFSETFACTOR_VALUE = 0.5;
double AnnotationTextStyle::GetSuperScriptOffsetFactor() const { return getRealValue(m_data, AnnotationTextStyleProperty::SuperScriptOffsetFactor, DEFAULT_SUPERSCRIPTOFFSETFACTOR_VALUE); }
void AnnotationTextStyle::SetSuperScriptOffsetFactor(double value) { setRealValue(m_data, AnnotationTextStyleProperty::SuperScriptOffsetFactor, DEFAULT_SUPERSCRIPTOFFSETFACTOR_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_SUPERSCRIPTSCALE_VALUE = (2.0 / 3.0);
double AnnotationTextStyle::GetSuperScriptScale() const { return getRealValue(m_data, AnnotationTextStyleProperty::SuperScriptScale, DEFAULT_SUPERSCRIPTSCALE_VALUE); }
void AnnotationTextStyle::SetSuperScriptScale(double value) { setRealValue(m_data, AnnotationTextStyleProperty::SuperScriptScale, DEFAULT_SUPERSCRIPTSCALE_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_WIDTHFACTOR_VALUE = 1.0;
double AnnotationTextStyle::GetWidthFactor() const { return getRealValue(m_data, AnnotationTextStyleProperty::WidthFactor, DEFAULT_WIDTHFACTOR_VALUE); }
void AnnotationTextStyle::SetWidthFactor(double value) { setRealValue(m_data, AnnotationTextStyleProperty::WidthFactor, DEFAULT_WIDTHFACTOR_VALUE, value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr AnnotationTextStyle::CreateEffectiveStyle(AnnotationTextStylePropertyBagCR overrides) const
    {
    AnnotationTextStylePtr copy = CreateCopy();
    copy->InvalidateElementId();
    copy->InvalidateCode();
    copy->m_description.clear();
    copy->m_data.MergeWith(overrides);

    return copy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr AnnotationTextStyle::CreateEffectiveStyle(
                                                                 AnnotationTextStyleCR docStyle, AnnotationTextStylePropertyBagCR docOverrides,
                                                                 AnnotationTextStyleCR parStyle, AnnotationTextStylePropertyBagCR parOverrides,
                                                                 AnnotationTextStyleCR runStyle, AnnotationTextStylePropertyBagCR runOverrides
                                                                 )
    {
    // Intelligently merge multiple levels of styles and overrides, where the effective value comes from the relevant level.

    BeAssert(&docStyle.GetDgnDb() == &parStyle.GetDgnDb());
    BeAssert(&docStyle.GetDgnDb() == &runStyle.GetDgnDb());

    AnnotationTextStylePtr effectiveStyle = AnnotationTextStyle::Create(docStyle.GetDgnDb());

    // Document
    effectiveStyle->SetHeight(docOverrides.HasProperty(AnnotationTextStyleProperty::Height) ? docOverrides.GetRealProperty(AnnotationTextStyleProperty::Height) : docStyle.GetHeight());
    effectiveStyle->SetLineSpacingFactor(docOverrides.HasProperty(AnnotationTextStyleProperty::LineSpacingFactor) ? docOverrides.GetRealProperty(AnnotationTextStyleProperty::LineSpacingFactor) : docStyle.GetLineSpacingFactor());
    effectiveStyle->SetWidthFactor(docOverrides.HasProperty(AnnotationTextStyleProperty::WidthFactor) ? docOverrides.GetRealProperty(AnnotationTextStyleProperty::WidthFactor) : docStyle.GetWidthFactor());

    // Paragraph
    //  --

    // Run
    effectiveStyle->SetColorType(runOverrides.HasProperty(AnnotationTextStyleProperty::ColorType) ? (AnnotationColorType)runOverrides.GetIntegerProperty(AnnotationTextStyleProperty::ColorType) : runStyle.GetColorType());
    effectiveStyle->SetColorValue(runOverrides.HasProperty(AnnotationTextStyleProperty::ColorValue) ? ColorDef((uint32_t)runOverrides.GetIntegerProperty(AnnotationTextStyleProperty::ColorValue)) : runStyle.GetColorValue());
    effectiveStyle->SetFontId(runOverrides.HasProperty(AnnotationTextStyleProperty::FontId) ? DgnFontId((uint64_t)runOverrides.GetIntegerProperty(AnnotationTextStyleProperty::FontId)) : runStyle.GetFontId());
    effectiveStyle->SetIsBold(runOverrides.HasProperty(AnnotationTextStyleProperty::IsBold) ? (0 != runOverrides.GetIntegerProperty(AnnotationTextStyleProperty::IsBold)) : runStyle.IsBold());
    effectiveStyle->SetIsItalic(runOverrides.HasProperty(AnnotationTextStyleProperty::IsItalic) ? (0 != runOverrides.GetIntegerProperty(AnnotationTextStyleProperty::IsItalic)) : runStyle.IsItalic());
    effectiveStyle->SetIsUnderlined(runOverrides.HasProperty(AnnotationTextStyleProperty::IsUnderlined) ? (0 != runOverrides.GetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined)) : runStyle.IsUnderlined());
    effectiveStyle->SetStackedFractionScale(runOverrides.HasProperty(AnnotationTextStyleProperty::StackedFractionScale) ? runOverrides.GetRealProperty(AnnotationTextStyleProperty::StackedFractionScale) : runStyle.GetStackedFractionScale());
    effectiveStyle->SetStackedFractionType(runOverrides.HasProperty(AnnotationTextStyleProperty::StackedFractionType) ? (AnnotationStackedFractionType)runOverrides.GetIntegerProperty(AnnotationTextStyleProperty::StackedFractionType) : runStyle.GetStackedFractionType());
    effectiveStyle->SetSubScriptOffsetFactor(runOverrides.HasProperty(AnnotationTextStyleProperty::SubScriptOffsetFactor) ? runOverrides.GetRealProperty(AnnotationTextStyleProperty::SubScriptOffsetFactor) : runStyle.GetSubScriptOffsetFactor());
    effectiveStyle->SetSubScriptScale(runOverrides.HasProperty(AnnotationTextStyleProperty::SubScriptScale) ? runOverrides.GetRealProperty(AnnotationTextStyleProperty::SubScriptScale) : runStyle.GetSubScriptScale());
    effectiveStyle->SetSuperScriptOffsetFactor(runOverrides.HasProperty(AnnotationTextStyleProperty::SuperScriptOffsetFactor) ? runOverrides.GetRealProperty(AnnotationTextStyleProperty::SuperScriptOffsetFactor) : runStyle.GetSuperScriptOffsetFactor());
    effectiveStyle->SetSuperScriptScale(runOverrides.HasProperty(AnnotationTextStyleProperty::SuperScriptScale) ? runOverrides.GetRealProperty(AnnotationTextStyleProperty::SuperScriptScale) : runStyle.GetSuperScriptScale());

    return effectiveStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
size_t AnnotationTextStyle::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr select = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationTextStyle));
    if (!select.IsValid())
        return 0;
    
    if (BE_SQLITE_ROW != select->Step())
        return 0;

    return static_cast<size_t>(select->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
AnnotationTextStyle::Iterator AnnotationTextStyle::MakeIterator(DgnDbR db)
    {
    Iterator iter;
    iter.Prepare(db, "SELECT ECInstanceId, Code.[Value], Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationTextStyle), 0);

    return iter;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const uint32_t CURRENT_MAJOR_VERSION = 2;
static const uint32_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendIntegerSetter
(
FB::AnnotationTextStyleSetters& setters,
AnnotationTextStylePropertyBagCR data,
AnnotationTextStyleProperty tsProp,
decltype(declval<FB::AnnotationTextStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationTextStyleSetter>().integerValue()) defaultValue,
bool writeIfDefault
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetIntegerProperty(tsProp);
    if (!writeIfDefault && (value == defaultValue))
        return;

    setters.push_back(FB::AnnotationTextStyleSetter(fbProp, value, 0.0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendRealSetter
(
FB::AnnotationTextStyleSetters& setters,
AnnotationTextStylePropertyBagCR data,
AnnotationTextStyleProperty tsProp,
decltype(declval<FB::AnnotationTextStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationTextStyleSetter>().realValue()) defaultValue,
bool writeIfDefault
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetRealProperty(tsProp);
    if (!writeIfDefault && (value == defaultValue))
        return;

    setters.push_back(FB::AnnotationTextStyleSetter(fbProp, 0, value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextStylePersistence::EncodeAsFlatBuf(FB::AnnotationTextStyleSetters& setters, AnnotationTextStylePropertyBagCR data) { return EncodeAsFlatBuf(setters, data, FlatBufEncodeOptions::Default); }
BentleyStatus AnnotationTextStylePersistence::EncodeAsFlatBuf(FB::AnnotationTextStyleSetters& setters, AnnotationTextStylePropertyBagCR data, FlatBufEncodeOptions options)
    {
    bool writeIfDefault = isEnumFlagSet(FlatBufEncodeOptions::SettersAreOverrides, options);

    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::ColorType, FB::AnnotationTextStyleProperty_ColorType, DEFAULT_COLOR_VALUE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::ColorValue, FB::AnnotationTextStyleProperty_ColorValue, DEFAULT_COLOR_TYPE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::FontId, FB::AnnotationTextStyleProperty_FontId, DEFAULT_FONTID_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::Height, FB::AnnotationTextStyleProperty_Height, DEFAULT_HEIGHT_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::LineSpacingFactor, FB::AnnotationTextStyleProperty_LineSpacingFactor, DEFAULT_LINESPACINGFACTOR_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsBold, FB::AnnotationTextStyleProperty_IsBold, DEFAULT_ISBOLD_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsItalic, FB::AnnotationTextStyleProperty_IsItalic, DEFAULT_ISITALIC_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsUnderlined, FB::AnnotationTextStyleProperty_IsUnderlined, DEFAULT_ISUNDERLINED_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::StackedFractionScale, FB::AnnotationTextStyleProperty_StackedFractionScale, DEFAULT_STACKEDFRACTIONSCALE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::StackedFractionType, FB::AnnotationTextStyleProperty_StackedFractionType, DEFAULT_STACKEDFRACTIONTYPE_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SubScriptOffsetFactor, FB::AnnotationTextStyleProperty_SubScriptOffsetFactor, DEFAULT_SUBSCRIPTOFFSETFACTOR_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SubScriptScale, FB::AnnotationTextStyleProperty_SubScriptScale, DEFAULT_SUBSCRIPTSCALE_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SuperScriptOffsetFactor, FB::AnnotationTextStyleProperty_SuperScriptOffsetFactor, DEFAULT_SUPERSCRIPTOFFSETFACTOR_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SuperScriptScale, FB::AnnotationTextStyleProperty_SuperScriptScale, DEFAULT_SUPERSCRIPTSCALE_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::WidthFactor, FB::AnnotationTextStyleProperty_WidthFactor, DEFAULT_WIDTHFACTOR_VALUE, writeIfDefault);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextStylePersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, AnnotationTextStyleCR style, FlatBufEncodeOptions options)
    {
    FlatBufferBuilder encoder;
    
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::AnnotationTextStyleSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data, options), ERROR);
    
    FB::AnnotationTextStyleSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);
    
    //.............................................................................................
    FB::AnnotationTextStyleBuilder fbStyle(encoder);
    fbStyle.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbStyle.add_minorVersion(CURRENT_MINOR_VERSION);

    if (!setters.empty())
        fbStyle.add_setters(settersOffset);
    
    encoder.Finish(fbStyle.Finish());

    //.............................................................................................
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextStylePersistence::DecodeFromFlatBuf(AnnotationTextStylePropertyBagR data, FB::AnnotationTextStyleSetterVector const& setters)
    {
    for (auto const& setter : setters)
        {
        switch (setter.key())
            {
            case FB::AnnotationTextStyleProperty_ColorType: data.SetIntegerProperty(AnnotationTextStyleProperty::ColorType, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_ColorValue: data.SetIntegerProperty(AnnotationTextStyleProperty::ColorValue, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_FontId: data.SetIntegerProperty(AnnotationTextStyleProperty::FontId, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_Height: data.SetRealProperty(AnnotationTextStyleProperty::Height, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_LineSpacingFactor: data.SetRealProperty(AnnotationTextStyleProperty::LineSpacingFactor, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_IsBold: data.SetIntegerProperty(AnnotationTextStyleProperty::IsBold, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_IsItalic: data.SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_IsUnderlined: data.SetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_StackedFractionScale: data.SetRealProperty(AnnotationTextStyleProperty::StackedFractionScale, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_StackedFractionType: data.SetIntegerProperty(AnnotationTextStyleProperty::StackedFractionType, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_SubScriptOffsetFactor: data.SetRealProperty(AnnotationTextStyleProperty::SubScriptOffsetFactor, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_SubScriptScale: data.SetRealProperty(AnnotationTextStyleProperty::SubScriptScale, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_SuperScriptOffsetFactor: data.SetRealProperty(AnnotationTextStyleProperty::SuperScriptOffsetFactor, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_SuperScriptScale: data.SetRealProperty(AnnotationTextStyleProperty::SuperScriptScale, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_WidthFactor: data.SetRealProperty(AnnotationTextStyleProperty::WidthFactor, setter.realValue()); break;
            }
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextStylePersistence::DecodeFromFlatBuf(AnnotationTextStyleR style, ByteCP buffer, size_t numBytes)
    {
    style.m_data.ClearAllProperties();
    
    auto fbStyle = GetRoot<FB::AnnotationTextStyle>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }
