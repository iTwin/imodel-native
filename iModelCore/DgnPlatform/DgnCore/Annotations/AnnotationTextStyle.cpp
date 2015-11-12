/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/AnnotationTextStyle.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
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
#define PROP_Descr "Descr"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(AnnotationTextStyleHandler);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTextStyleHandler::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(PROP_Data);
    params.Add(PROP_Descr);
    }
}
END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextStyle::CreateParams::CreateParams(DgnDbR db, Utf8StringCR name, AnnotationTextStylePropertyBagCR data, Utf8StringCR descr)
    : T_Super(db, QueryDgnClassId(db), CreateStyleCode(name)), m_data(data), m_descr(descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationTextStyle::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationTextStylePersistence::EncodeAsFlatBuf(data, *this, AnnotationTextStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), DgnDbStatus::BadArg);

    return ECSqlStatus::Success == stmt.BindText(stmt.GetParameterIndex(PROP_Descr), m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        && ECSqlStatus::Success == stmt.BindBinary(stmt.GetParameterIndex(PROP_Data), &data[0], static_cast<int>(data.size()), IECSqlBinder::MakeCopy::Yes)
        ? DgnDbStatus::Success : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationTextStyle::_ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        int dataSize = 0;
        Byte const* data = static_cast<Byte const*>(stmt.GetValueBinary(params.GetSelectIndex(PROP_Data), &dataSize));
        POSTCONDITION(SUCCESS == AnnotationTextStylePersistence::DecodeFromFlatBuf(*this, data, static_cast<size_t>(dataSize)), DgnDbStatus::BadArg);

        m_descr.AssignOrClear(stmt.GetValueText(params.GetSelectIndex(PROP_Descr)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationTextStyle::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationTextStyle::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTextStyle::_CopyFrom(DgnElementCR src)
    {
    T_Super::_CopyFrom(src);
    auto other = dynamic_cast<AnnotationTextStyleCP>(&src);
    BeAssert(nullptr != other);
    if (nullptr != other)
        {
        m_descr = other->m_descr;
        m_data = other->m_data;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationTextStyle::_OnDelete() const
    {
    return DgnDbStatus::DeletionProhibited; // purge only
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId AnnotationTextStyle::QueryStyleId(Code const& code, DgnDbR db)
    {
    return DgnElementId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationTextStyle::Reset()
    {
    InvalidateElementId();
    InvalidateCode();
    m_descr.clear();
    ResetProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTextStyle::ResetProperties()
    {
    m_data.ClearAllProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool AnnotationTextStyle::ExistsById(DgnElementId id, DgnDbR db)
    {
    PRECONDITION(id.IsValid(), false);

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationTextStyle) " WHERE ECInstanceId=? LIMIT 1");
    if (stmt.IsValid())
        {
        stmt->BindId(1, id);
        if (BE_SQLITE_ROW == stmt->Step())
            return 0 < stmt->GetValueInt(0);
        }

    return false;
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
    AnnotationTextStylePtr clone = Clone();
    clone->InvalidateCode();
    clone->m_descr.clear();
    clone->InvalidateElementId();

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
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::ColorType, FB::AnnotationTextStyleProperty_ColorType, DEFAULT_COLOR_VALUE_VALUE);
    appendIntegerSetter(setters, data, AnnotationTextStyleProperty::ColorValue, FB::AnnotationTextStyleProperty_ColorValue, DEFAULT_COLOR_TYPE_VALUE);
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
    FB::AnnotationTextStyleSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data), ERROR);
    
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
    style.ResetProperties();
    
    auto fbStyle = GetRoot<FB::AnnotationTextStyle>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AnnotationTextStyle::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationTextStyle));
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? static_cast<size_t>(stmt->GetValueInt(0)) : 0;
    }

#define SELECT_AnnotationTextStyle "SELECT ECInstanceId, Code, Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationTextStyle)
#define SELECT_ORDERED_AnnotationTextStyle SELECT_AnnotationTextStyle " ORDER BY Code"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextStyle::Iterator AnnotationTextStyle::MakeIterator(DgnDbR db, bool ordered)
    {
    Utf8CP ecSql = ordered? SELECT_ORDERED_AnnotationTextStyle : SELECT_AnnotationTextStyle;

    Iterator iter;
    iter.Prepare(db, ecSql, 0);

    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCR AnnotationTextStyle::ResolveFont() const
    {
    return DgnFontManager::ResolveFont(GetDgnDb().Fonts().FindFontById(GetFontId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t AnnotationTextStyle::_GetMemSize() const
    {
    return T_Super::_GetMemSize() + m_data.GetMemSize() + static_cast<uint32_t>(m_descr.length());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t AnnotationPropertyBag::GetMemSize() const
    {
    return sizeof(*this);   // NEEDSWORK accurate mem size
    }

