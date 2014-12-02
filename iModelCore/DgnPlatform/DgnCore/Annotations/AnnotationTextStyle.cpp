//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationTextStyle.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationTextStylePropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((AnnotationTextStyleProperty)key)
        {
        case AnnotationTextStyleProperty::ColorId:
        case AnnotationTextStyleProperty::FontId:
        case AnnotationTextStyleProperty::IsBold:
        case AnnotationTextStyleProperty::IsItalic:
        case AnnotationTextStyleProperty::IsSubScript:
        case AnnotationTextStyleProperty::IsSuperScript:
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePropertyBagPtr AnnotationTextStylePropertyBag::Create() { return new AnnotationTextStylePropertyBag(); }
AnnotationTextStylePropertyBag::AnnotationTextStylePropertyBag() :
    T_Super()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePropertyBagPtr AnnotationTextStylePropertyBag::Clone() const { return new AnnotationTextStylePropertyBag(*this); }
AnnotationTextStylePropertyBag::AnnotationTextStylePropertyBag(AnnotationTextStylePropertyBagCR rhs) : T_Super(rhs) { }
AnnotationTextStylePropertyBagR AnnotationTextStylePropertyBag::operator=(AnnotationTextStylePropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationTextStylePropertyBag::HasProperty(AnnotationTextStyleProperty key) const { return T_Super::HasProperty((T_Key)key); }
void AnnotationTextStylePropertyBag::ClearProperty(AnnotationTextStyleProperty key) { T_Super::ClearProperty((T_Key)key); }
AnnotationTextStylePropertyBag::T_Integer AnnotationTextStylePropertyBag::GetIntegerProperty(AnnotationTextStyleProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
void AnnotationTextStylePropertyBag::SetIntegerProperty(AnnotationTextStyleProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
AnnotationTextStylePropertyBag::T_Real AnnotationTextStylePropertyBag::GetRealProperty(AnnotationTextStyleProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
void AnnotationTextStylePropertyBag::SetRealProperty(AnnotationTextStyleProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr AnnotationTextStyle::Create(DgnProjectR project) { return new AnnotationTextStyle(project); }
AnnotationTextStyle::AnnotationTextStyle(DgnProjectR project) :
    T_Super()
    {
    m_project = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr AnnotationTextStyle::Clone() const { return new AnnotationTextStyle(*this); }
AnnotationTextStyle::AnnotationTextStyle(AnnotationTextStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationTextStyleR AnnotationTextStyle::operator=(AnnotationTextStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationTextStyle::CopyFrom(AnnotationTextStyleCR rhs)
    {
    m_project = rhs.m_project;
    m_id = rhs.m_id;
    m_name = rhs.m_name;
    m_description = rhs.m_description;
    m_data = rhs.m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextStyle::Reset()
    {
    m_id.Invalidate();
    m_name.clear();
    m_description.clear();
    m_data.ClearAllProperties();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
DgnProjectR AnnotationTextStyle::GetDgnProjectR() const { return *m_project; }
DgnStyleId AnnotationTextStyle::GetId() const { return m_id; }
void AnnotationTextStyle::SetId(DgnStyleId value) { m_id = value; }
Utf8StringCR AnnotationTextStyle::GetName() const { return m_name; }
void AnnotationTextStyle::SetName(Utf8CP value) { m_name = value; }
Utf8StringCR AnnotationTextStyle::GetDescription() const { return m_description; }
void AnnotationTextStyle::SetDescription(Utf8CP value) { m_description = value; }

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
static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_COLORID_VALUE = 0;
UInt32 AnnotationTextStyle::GetColorId() const { return (UInt32)getIntegerValue(m_data, AnnotationTextStyleProperty::ColorId, DEFAULT_COLORID_VALUE); }
void AnnotationTextStyle::SetColorId(UInt32 value) { setIntegerValue(m_data, AnnotationTextStyleProperty::ColorId, DEFAULT_COLORID_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_FONTID_VALUE = 0;
UInt32 AnnotationTextStyle::GetFontId() const { return (UInt32)getIntegerValue(m_data, AnnotationTextStyleProperty::FontId, DEFAULT_FONTID_VALUE); }
void AnnotationTextStyle::SetFontId(UInt32 value) { setIntegerValue(m_data, AnnotationTextStyleProperty::FontId, DEFAULT_FONTID_VALUE, value); }

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

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_ISSUBSCRIPT_VALUE = 0;
bool AnnotationTextStyle::IsSubScript() const { return (0 != getIntegerValue(m_data, AnnotationTextStyleProperty::IsSubScript, DEFAULT_ISSUBSCRIPT_VALUE)); }
void AnnotationTextStyle::SetIsSubScript(bool value) { setIntegerValue(m_data, AnnotationTextStyleProperty::IsSubScript, DEFAULT_ISSUBSCRIPT_VALUE, (value ? 1 : 0)); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_ISSUPERSCRIPT_VALUE = 0;
bool AnnotationTextStyle::IsSuperScript() const { return (0 != getIntegerValue(m_data, AnnotationTextStyleProperty::IsSuperScript, DEFAULT_ISSUPERSCRIPT_VALUE)); }
void AnnotationTextStyle::SetIsSuperScript(bool value) { setIntegerValue(m_data, AnnotationTextStyleProperty::IsSuperScript, DEFAULT_ISSUPERSCRIPT_VALUE, (value ? 1 : 0)); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_ISUNDERLINED_VALUE = 0;
bool AnnotationTextStyle::IsUnderlined() const { return (0 != getIntegerValue(m_data, AnnotationTextStyleProperty::IsUnderlined, DEFAULT_ISUNDERLINED_VALUE)); }
void AnnotationTextStyle::SetIsUnderlined(bool value) { setIntegerValue(m_data, AnnotationTextStyleProperty::IsUnderlined, DEFAULT_ISUNDERLINED_VALUE, (value ? 1 : 0)); }

static const AnnotationTextStylePropertyBag::T_Real DEFAULT_STACKEDFRACTIONSCALE_VALUE = 0.7;
double AnnotationTextStyle::GetStackedFractionScale() const { return getRealValue(m_data, AnnotationTextStyleProperty::StackedFractionScale, DEFAULT_STACKEDFRACTIONSCALE_VALUE); }
void AnnotationTextStyle::SetStackedFractionScale(double value) { setRealValue(m_data, AnnotationTextStyleProperty::StackedFractionScale, DEFAULT_STACKEDFRACTIONSCALE_VALUE, value); }

static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_STACKEDFRACTIONTYPE_VALUE = (AnnotationTextStylePropertyBag::T_Integer)AnnotationStackedFractionType::HorizontalBar;
AnnotationStackedFractionType AnnotationTextStyle::GetStackedFractionType() const { return (AnnotationStackedFractionType)getIntegerValue(m_data, AnnotationTextStyleProperty::StackedFractionType, DEFAULT_STACKEDFRACTIONTYPE_VALUE); }
void AnnotationTextStyle::SetStackedFractionType(AnnotationStackedFractionType value) { setIntegerValue(m_data, AnnotationTextStyleProperty::StackedFractionType, DEFAULT_STACKEDFRACTIONTYPE_VALUE, (UInt32)value); }

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
    AnnotationTextStylePtr clone = Clone();
    clone->m_name.clear();
    clone->m_description.clear();
    clone->m_id.Invalidate();

    clone->m_data.MergeWith(overrides);

    return clone;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const UInt32 CURRENT_MAJOR_VERSION = 1;
static const UInt32 CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendIntegerSetter
(
FB::AnnotationTextStyleSetters& setters,
AnnotationTextStylePropertyBagCR data,
AnnotationTextStyleProperty tsProp,
decltype(declval<FB::AnnotationTextStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationTextStyleSetter>().integerValue()) defaultValue
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetIntegerProperty(tsProp);
    if (value == defaultValue)
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
decltype(declval<FB::AnnotationTextStyleSetter>().realValue()) defaultValue
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetRealProperty(tsProp);
    if (value == defaultValue)
        return;

    setters.push_back(FB::AnnotationTextStyleSetter(fbProp, 0, value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTextStylePersistence::EncodeAsFlatBuf(FB::AnnotationTextStyleSetters& setters, AnnotationTextStylePropertyBagCR data)
    {
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::ColorId, FB::AnnotationTextStyleProperty_ColorId, DEFAULT_COLORID_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::FontId, FB::AnnotationTextStyleProperty_FontId, DEFAULT_FONTID_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::Height, FB::AnnotationTextStyleProperty_Height, DEFAULT_HEIGHT_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::LineSpacingFactor, FB::AnnotationTextStyleProperty_LineSpacingFactor, DEFAULT_LINESPACINGFACTOR_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsBold, FB::AnnotationTextStyleProperty_IsBold, DEFAULT_ISBOLD_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsItalic, FB::AnnotationTextStyleProperty_IsItalic, DEFAULT_ISITALIC_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsSubScript, FB::AnnotationTextStyleProperty_IsSubScript, DEFAULT_ISSUBSCRIPT_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsSuperScript, FB::AnnotationTextStyleProperty_IsSuperScript, DEFAULT_ISSUPERSCRIPT_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsUnderlined, FB::AnnotationTextStyleProperty_IsUnderlined, DEFAULT_ISUNDERLINED_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::StackedFractionScale, FB::AnnotationTextStyleProperty_StackedFractionScale, DEFAULT_STACKEDFRACTIONSCALE_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::StackedFractionType, FB::AnnotationTextStyleProperty_StackedFractionType, DEFAULT_STACKEDFRACTIONTYPE_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SubScriptOffsetFactor, FB::AnnotationTextStyleProperty_SubScriptOffsetFactor, DEFAULT_SUBSCRIPTOFFSETFACTOR_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SubScriptScale, FB::AnnotationTextStyleProperty_SubScriptScale, DEFAULT_SUBSCRIPTSCALE_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SuperScriptOffsetFactor, FB::AnnotationTextStyleProperty_SuperScriptOffsetFactor, DEFAULT_SUPERSCRIPTOFFSETFACTOR_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::SuperScriptScale, FB::AnnotationTextStyleProperty_SuperScriptScale, DEFAULT_SUPERSCRIPTSCALE_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::WidthFactor, FB::AnnotationTextStyleProperty_WidthFactor, DEFAULT_WIDTHFACTOR_VALUE);
    
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
    Offset<String> nameOffset;
    Offset<String> descriptionOffset;
    if (!isEnumFlagSet(FlatBufEncodeOptions::ExcludeNonPropertyData, options))
        {
        PRECONDITION(!style.m_name.empty(), ERROR);

        nameOffset = encoder.CreateString(style.m_name);
        
        if (!style.m_description.empty())
            descriptionOffset = encoder.CreateString(style.m_description);
        }

    FB::AnnotationTextStyleSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data), ERROR);
    
    FB::AnnotationTextStyleSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);
    
    //.............................................................................................
    FB::AnnotationTextStyleBuilder fbStyle(encoder);
    fbStyle.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbStyle.add_minorVersion(CURRENT_MINOR_VERSION);

    if (!isEnumFlagSet(FlatBufEncodeOptions::ExcludeNonPropertyData, options))
        {
        fbStyle.add_id(style.GetId().GetValue());
        fbStyle.add_name(nameOffset);

        if (!style.m_description.empty())
            fbStyle.add_description(descriptionOffset);
        }
    
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
            case FB::AnnotationTextStyleProperty_ColorId: data.SetIntegerProperty(AnnotationTextStyleProperty::ColorId, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_FontId: data.SetIntegerProperty(AnnotationTextStyleProperty::FontId, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_Height: data.SetRealProperty(AnnotationTextStyleProperty::Height, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_LineSpacingFactor: data.SetRealProperty(AnnotationTextStyleProperty::LineSpacingFactor, setter.realValue()); break;
            case FB::AnnotationTextStyleProperty_IsBold: data.SetIntegerProperty(AnnotationTextStyleProperty::IsBold, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_IsItalic: data.SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_IsSubScript: data.SetIntegerProperty(AnnotationTextStyleProperty::IsSubScript, setter.integerValue()); break;
            case FB::AnnotationTextStyleProperty_IsSuperScript: data.SetIntegerProperty(AnnotationTextStyleProperty::IsSuperScript, setter.integerValue()); break;
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
    style.Reset();
    
    auto fbStyle = GetRoot<FB::AnnotationTextStyle>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_id())
        style.m_id = DgnStyleId(fbStyle->id());

    if (fbStyle->has_name())
        style.m_name = fbStyle->name()->c_str();

    if (fbStyle->has_description())
        style.m_description = fbStyle->description()->c_str();

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr DgnAnnotationTextStyles::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), NULL);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::AnnotationText, id);
    if (!styleRow.GetId().IsValid())
        return NULL;

    AnnotationTextStylePtr style = AnnotationTextStyle::Create(m_project);
    
    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Therefore, do this before setting the fields from the table.
    POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)styleRow.GetData(), styleRow.GetDataSize()), NULL);
    
    style->SetDescription(styleRow.GetDescription());
    style->SetId(id);
    style->SetName(styleRow.GetName());

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr DgnAnnotationTextStyles::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), NULL);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::AnnotationText, name);
    if (!id.IsValid())
        return NULL;

    return QueryById(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
bool DgnAnnotationTextStyles::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::AnnotationText, id);
    
    return styleRow.GetId().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
bool DgnAnnotationTextStyles::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::AnnotationText, name);
    
    return id.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr DgnAnnotationTextStyles::Insert(AnnotationTextStyleCR style)
    {
    PRECONDITION(!style.GetName().empty(), NULL);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, NULL);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::EncodeAsFlatBuf(styleData, style, AnnotationTextStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), NULL);

    DgnStyles::Style styleRow(DgnStyleId(), DgnStyleType::AnnotationText, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().InsertStyle(styleRow)))
        return NULL;

    return QueryById(styleRow.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationTextStyles::Update(AnnotationTextStyleCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    PRECONDITION(!style.GetName().empty(), ERROR);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, ERROR);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::EncodeAsFlatBuf(styleData, style, AnnotationTextStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyles::Style styleRow(style.GetId(), DgnStyleType::AnnotationText, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().UpdateStyle(styleRow)))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationTextStyles::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);
    
    POSTCONDITION(BE_SQLITE_DONE == m_project.Styles().DeleteStyle(DgnStyleType::AnnotationText, id), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
DgnStyles::Iterator DgnAnnotationTextStyles::MakeIterator(DgnStyleSort sortOrder) const
    {
    Utf8String queryModifierClauses;
    queryModifierClauses.Sprintf("WHERE Type=%d", DgnStyleType::AnnotationText);

    switch (sortOrder)
        {
        case DgnStyleSort::None:       break;
        case DgnStyleSort::NameAsc:    queryModifierClauses += " ORDER BY Name ASC";   break;
        case DgnStyleSort::NameDsc:    queryModifierClauses += " ORDER BY Name DESC";  break;

        default:
            BeAssert(false); // Unknown/unexpected DgnStyleSort
            break;
        }

    DgnStyles::Iterator it(m_project);
    it.Params().SetWhere(queryModifierClauses.c_str());
    return it;
    }
