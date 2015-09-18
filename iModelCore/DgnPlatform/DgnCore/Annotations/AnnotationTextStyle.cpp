/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/AnnotationTextStyle.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

#define DGN_STYLE_TYPE_AnnotationText "2"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
bool AnnotationTextStylePropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((AnnotationTextStyleProperty)key)
        {
        case AnnotationTextStyleProperty::Color:
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextStyle::AnnotationTextStyle(DgnDbR project) :
    T_Super()
    {
    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextStyle::CopyFrom(AnnotationTextStyleCR rhs)
    {
    m_dgndb = rhs.m_dgndb;
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
static const AnnotationTextStylePropertyBag::T_Integer DEFAULT_COLOR_VALUE = ElementColor().ToInt64();
ElementColor AnnotationTextStyle::GetColor() const { return ElementColor(getIntegerValue(m_data, AnnotationTextStyleProperty::Color, DEFAULT_COLOR_VALUE)); }
void AnnotationTextStyle::SetColor(ElementColor value) { setIntegerValue(m_data, AnnotationTextStyleProperty::Color, DEFAULT_COLOR_VALUE, value.ToInt64()); }

static const int64_t DEFAULT_FONTID_VALUE = 0; // See definition of BeServerIssuedId, of which DgnFontId is a sub-class.
DgnFontId AnnotationTextStyle::GetFontId() const { return DgnFontId((int64_t)getIntegerValue(m_data, AnnotationTextStyleProperty::FontId, DEFAULT_FONTID_VALUE)); }
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
    AnnotationTextStylePtr clone = Clone();
    clone->m_name.clear();
    clone->m_description.clear();
    clone->m_id.Invalidate();

    clone->m_data.MergeWith(overrides);

    return clone;
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
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::Color, FB::AnnotationTextStyleProperty_Color, DEFAULT_COLOR_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::FontId, FB::AnnotationTextStyleProperty_FontId, DEFAULT_FONTID_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::Height, FB::AnnotationTextStyleProperty_Height, DEFAULT_HEIGHT_VALUE);
    appendRealSetter(setters, data, AnnotationTextStyleProperty::LineSpacingFactor, FB::AnnotationTextStyleProperty_LineSpacingFactor, DEFAULT_LINESPACINGFACTOR_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsBold, FB::AnnotationTextStyleProperty_IsBold, DEFAULT_ISBOLD_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::IsItalic, FB::AnnotationTextStyleProperty_IsItalic, DEFAULT_ISITALIC_VALUE);
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
            case FB::AnnotationTextStyleProperty_Color: data.SetIntegerProperty(AnnotationTextStyleProperty::Color, setter.integerValue()); break;
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


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr DgnAnnotationTextStyles::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Name,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationText " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    AnnotationTextStylePtr style = AnnotationTextStyle::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);

    style->SetDescription(query.GetValueText(1));
    style->SetId(id);
    style->SetName(query.GetValueText(0));

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr DgnAnnotationTextStyles::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Id,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationText " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    AnnotationTextStylePtr style = AnnotationTextStyle::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);
    
    style->SetDescription(query.GetValueText(1));
    style->SetId(query.GetValueId<DgnStyleId>(0));
    style->SetName(name);

    return style;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
DgnAnnotationTextStyles::Iterator::const_iterator DgnAnnotationTextStyles::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,Name,Descr FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationText, true);
        m_db->GetCachedStatement(m_stmt, sql.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), (BE_SQLITE_ROW == m_stmt->Step()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
size_t DgnAnnotationTextStyles::Iterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString("SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationText, true);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    m_params.Bind(query);

    POSTCONDITION(BE_SQLITE_ROW == query.Step(), 0);

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnAnnotationTextStyles::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationText " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return false;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnAnnotationTextStyles::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationText " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return false;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationTextStyles::Insert(AnnotationTextStyleR style)
    {
    // Don't assert to ensure an invalid ID.
    // Consider the case of cloning a style object, modifying, and then inserting it as a new style. The Clone keeps the ID, and I don't think it's worth having an overload of Clone to expose this detail.

    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationTextStylePersistence::EncodeAsFlatBuf(data, style, AnnotationTextStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyleId nextId;
    PRECONDITION(BE_SQLITE_OK == m_dgndb.GetServerIssuedId(nextId, DGN_TABLE(DGN_CLASSNAME_Style), "Id"), ERROR);
    
    Statement insert;
    insert.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Style) " (Id,Type,Name,Descr,Data) VALUES (?," DGN_STYLE_TYPE_AnnotationText ",?,?,?)");
    insert.BindId(1, nextId);
    insert.BindText(2, style.GetName().c_str(), Statement::MakeCopy::No);
    insert.BindText(3, style.GetDescription().c_str(), Statement::MakeCopy::No);
    insert.BindBlob(4, (void const*)&data[0], (int)data.size(), Statement::MakeCopy::No);

    POSTCONDITION(BE_SQLITE_DONE == insert.Step(), ERROR);

    style.SetId(nextId);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationTextStyles::Update(AnnotationTextStyleCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    
    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationTextStylePersistence::EncodeAsFlatBuf(data, style, AnnotationTextStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    Statement update;
    update.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Style) " SET Name=?,Descr=?,Data=? WHERE Type=" DGN_STYLE_TYPE_AnnotationText " AND Id=?");
    update.BindText(1, style.GetName().c_str(), Statement::MakeCopy::No);
    update.BindText(2, style.GetDescription().c_str(), Statement::MakeCopy::No);
    update.BindBlob(3, (void const*)&data[0], (int)data.size(), Statement::MakeCopy::No);
    update.BindId(4, style.GetId());

    POSTCONDITION(BE_SQLITE_DONE == update.Step(), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationTextStyles::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);

    Statement del;
    del.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationText " AND Id=?");
    del.BindId(1, id);

    POSTCONDITION(BE_SQLITE_DONE == del.Step(), ERROR);

    return SUCCESS;
    }
