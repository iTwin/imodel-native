//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationFrameStyle.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFrameStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationFrameStylePropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((AnnotationFrameStyleProperty)key)
        {
        case AnnotationFrameStyleProperty::FillColorId:
        case AnnotationFrameStyleProperty::IsFillEnabled:
        case AnnotationFrameStyleProperty::IsStrokeCloud:
        case AnnotationFrameStyleProperty::IsStrokeEnabled:
        case AnnotationFrameStyleProperty::StrokeColorId:
        case AnnotationFrameStyleProperty::StrokeStyle:
        case AnnotationFrameStyleProperty::StrokeWeight:
        case AnnotationFrameStyleProperty::Type:
            return true;

        default:
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationFrameStylePropertyBag::_IsRealProperty(T_Key key) const
    {
    switch ((AnnotationFrameStyleProperty)key)
        {
        case AnnotationFrameStyleProperty::CloudBulgeFactor:
        case AnnotationFrameStyleProperty::CloudDiameterFactor:
        case AnnotationFrameStyleProperty::FillTransparency:
        case AnnotationFrameStyleProperty::HorizontalPadding:
        case AnnotationFrameStyleProperty::VerticalPadding:
            return true;

        default:
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePropertyBagPtr AnnotationFrameStylePropertyBag::Create() { return new AnnotationFrameStylePropertyBag(); }
AnnotationFrameStylePropertyBag::AnnotationFrameStylePropertyBag() :
    T_Super()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePropertyBagPtr AnnotationFrameStylePropertyBag::Clone() const { return new AnnotationFrameStylePropertyBag(*this); }
AnnotationFrameStylePropertyBag::AnnotationFrameStylePropertyBag(AnnotationFrameStylePropertyBagCR rhs) : T_Super(rhs) { }
AnnotationFrameStylePropertyBagR AnnotationFrameStylePropertyBag::operator=(AnnotationFrameStylePropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationFrameStylePropertyBag::HasProperty(AnnotationFrameStyleProperty key) const { return T_Super::HasProperty((T_Key)key); }
void AnnotationFrameStylePropertyBag::ClearProperty(AnnotationFrameStyleProperty key) { T_Super::ClearProperty((T_Key)key); }
AnnotationFrameStylePropertyBag::T_Integer AnnotationFrameStylePropertyBag::GetIntegerProperty(AnnotationFrameStyleProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
void AnnotationFrameStylePropertyBag::SetIntegerProperty(AnnotationFrameStyleProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
AnnotationFrameStylePropertyBag::T_Real AnnotationFrameStylePropertyBag::GetRealProperty(AnnotationFrameStyleProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
void AnnotationFrameStylePropertyBag::SetRealProperty(AnnotationFrameStyleProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr AnnotationFrameStyle::Create(DgnDbR project) { return new AnnotationFrameStyle(project); }
AnnotationFrameStyle::AnnotationFrameStyle(DgnDbR project) :
    T_Super()
    {
    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr AnnotationFrameStyle::Clone() const { return new AnnotationFrameStyle(*this); }
AnnotationFrameStyle::AnnotationFrameStyle(AnnotationFrameStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationFrameStyleR AnnotationFrameStyle::operator=(AnnotationFrameStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationFrameStyle::CopyFrom(AnnotationFrameStyleCR rhs)
    {
    m_dgndb = rhs.m_dgndb;
    m_id = rhs.m_id;
    m_name = rhs.m_name;
    m_description = rhs.m_description;
    m_data = rhs.m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameStyle::Reset()
    {
    m_id.Invalidate();
    m_name.clear();
    m_description.clear();
    m_data.ClearAllProperties();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
DgnDbR AnnotationFrameStyle::GetDgnProjectR() const { return *m_dgndb; }
DgnStyleId AnnotationFrameStyle::GetId() const { return m_id; }
void AnnotationFrameStyle::SetId(DgnStyleId value) { m_id = value; }
Utf8StringCR AnnotationFrameStyle::GetName() const { return m_name; }
void AnnotationFrameStyle::SetName(Utf8CP value) { m_name = value; }
Utf8StringCR AnnotationFrameStyle::GetDescription() const { return m_description; }
void AnnotationFrameStyle::SetDescription(Utf8CP value) { m_description = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static AnnotationFrameStylePropertyBag::T_Integer getIntegerValue(AnnotationFrameStylePropertyBagCR data, AnnotationFrameStyleProperty key, AnnotationFrameStylePropertyBag::T_Integer defaultValue)
    {
    if (data.HasProperty(key))
        return data.GetIntegerProperty(key);

    return defaultValue;
    }
static void setIntegerValue(AnnotationFrameStylePropertyBagR data, AnnotationFrameStyleProperty key, AnnotationFrameStylePropertyBag::T_Integer defaultValue, AnnotationFrameStylePropertyBag::T_Integer value)
    {
    if (value == defaultValue)
        data.ClearProperty(key);
    else
        data.SetIntegerProperty(key, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static AnnotationFrameStylePropertyBag::T_Real getRealValue(AnnotationFrameStylePropertyBagCR data, AnnotationFrameStyleProperty key, AnnotationFrameStylePropertyBag::T_Real defaultValue)
    {
    if (data.HasProperty(key))
        return data.GetRealProperty(key);

    return defaultValue;
    }
static void setRealValue(AnnotationFrameStylePropertyBagR data, AnnotationFrameStyleProperty key, AnnotationFrameStylePropertyBag::T_Real defaultValue, AnnotationFrameStylePropertyBag::T_Real value)
    {
    if (value == defaultValue)
        data.ClearProperty(key);
    else
        data.SetRealProperty(key, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static const AnnotationFrameStylePropertyBag::T_Real DEFAULT_CLOUDBULGEFACTOR_VALUE = 1.1;
double AnnotationFrameStyle::GetCloudBulgeFactor() const { return getRealValue(m_data, AnnotationFrameStyleProperty::CloudBulgeFactor, DEFAULT_CLOUDBULGEFACTOR_VALUE); }
void AnnotationFrameStyle::SetCloudBulgeFactor(double value) { setRealValue(m_data, AnnotationFrameStyleProperty::CloudBulgeFactor, DEFAULT_CLOUDBULGEFACTOR_VALUE, value); }

static const AnnotationFrameStylePropertyBag::T_Real DEFAULT_CLOUDDIAMETERFACTOR_VALUE = 1.0;
double AnnotationFrameStyle::GetCloudDiameterFactor() const { return getRealValue(m_data, AnnotationFrameStyleProperty::CloudDiameterFactor, DEFAULT_CLOUDDIAMETERFACTOR_VALUE); }
void AnnotationFrameStyle::SetCloudDiameterFactor(double value) { setRealValue(m_data, AnnotationFrameStyleProperty::CloudDiameterFactor, DEFAULT_CLOUDDIAMETERFACTOR_VALUE, value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_FILLCOLORID_VALUE = 0;
ColorDef AnnotationFrameStyle::GetFillColor() const { return ColorDef((uint32_t) getIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorId, DEFAULT_FILLCOLORID_VALUE)); }
void AnnotationFrameStyle::SetFillColor(ColorDef value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorId, DEFAULT_FILLCOLORID_VALUE, value.GetValue()); }

static const AnnotationFrameStylePropertyBag::T_Real DEFAULT_FILLTRANSPARENCY_VALUE = 0.0;
double AnnotationFrameStyle::GetFillTransparency() const { return getRealValue(m_data, AnnotationFrameStyleProperty::FillTransparency, DEFAULT_FILLTRANSPARENCY_VALUE); }
void AnnotationFrameStyle::SetFillTransparency(double value) { setRealValue(m_data, AnnotationFrameStyleProperty::FillTransparency, DEFAULT_FILLTRANSPARENCY_VALUE, value); }

static const AnnotationFrameStylePropertyBag::T_Real DEFAULT_HORIZONTALPADDING_VALUE = 0.0;
double AnnotationFrameStyle::GetHorizontalPadding() const { return getRealValue(m_data, AnnotationFrameStyleProperty::HorizontalPadding, DEFAULT_HORIZONTALPADDING_VALUE); }
void AnnotationFrameStyle::SetHorizontalPadding(double value) { setRealValue(m_data, AnnotationFrameStyleProperty::HorizontalPadding, DEFAULT_HORIZONTALPADDING_VALUE, value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_ISFILLENABLED_VALUE = 0;
bool AnnotationFrameStyle::IsFillEnabled() const { return (0 != getIntegerValue(m_data, AnnotationFrameStyleProperty::IsFillEnabled, DEFAULT_ISFILLENABLED_VALUE)); }
void AnnotationFrameStyle::SetIsFillEnabled(bool value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::IsFillEnabled, DEFAULT_ISFILLENABLED_VALUE, (value ? 1 : 0)); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_ISSTROKECLOUD_VALUE = 0;
bool AnnotationFrameStyle::IsStrokeCloud() const { return (0 != getIntegerValue(m_data, AnnotationFrameStyleProperty::IsStrokeCloud, DEFAULT_ISSTROKECLOUD_VALUE)); }
void AnnotationFrameStyle::SetIsStrokeCloud(bool value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::IsStrokeCloud, DEFAULT_ISSTROKECLOUD_VALUE, (value ? 1 : 0)); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_ISSTROKEENABLED_VALUE = 0;
bool AnnotationFrameStyle::IsStrokeEnabled() const { return (0 != getIntegerValue(m_data, AnnotationFrameStyleProperty::IsStrokeEnabled, DEFAULT_ISSTROKEENABLED_VALUE)); }
void AnnotationFrameStyle::SetIsStrokeEnabled(bool value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::IsStrokeEnabled, DEFAULT_ISSTROKEENABLED_VALUE, (value ? 1 : 0)); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_STROKECOLORID_VALUE = 0;
ColorDef AnnotationFrameStyle::GetStrokeColor() const { return ColorDef((uint32_t)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorId, DEFAULT_STROKECOLORID_VALUE)); }
void AnnotationFrameStyle::SetStrokeColor(ColorDef value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorId, DEFAULT_STROKECOLORID_VALUE, value.GetValue()); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_STROKESTYLE_VALUE = 0;
int32_t AnnotationFrameStyle::GetStrokeStyle() const { return (int32_t)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeStyle, DEFAULT_STROKESTYLE_VALUE); }
void AnnotationFrameStyle::SetStrokeStyle(int32_t value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeStyle, DEFAULT_STROKESTYLE_VALUE, (uint32_t)value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_STROKEWEIGHT_VALUE = 0;
uint32_t AnnotationFrameStyle::GetStrokeWeight() const { return (uint32_t)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeWeight, DEFAULT_STROKEWEIGHT_VALUE); }
void AnnotationFrameStyle::SetStrokeWeight(uint32_t value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeWeight, DEFAULT_STROKEWEIGHT_VALUE, value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_TYPE_VALUE = (AnnotationFrameStylePropertyBag::T_Integer)AnnotationFrameType::InvisibleBox;
AnnotationFrameType AnnotationFrameStyle::GetType() const { return (AnnotationFrameType)getIntegerValue(m_data, AnnotationFrameStyleProperty::Type, DEFAULT_TYPE_VALUE); }
void AnnotationFrameStyle::SetType(AnnotationFrameType value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::Type, DEFAULT_TYPE_VALUE, (uint32_t)value); }

static const AnnotationFrameStylePropertyBag::T_Real DEFAULT_VERTICALPADDING_VALUE = 0.0;
double AnnotationFrameStyle::GetVerticalPadding() const { return getRealValue(m_data, AnnotationFrameStyleProperty::VerticalPadding, DEFAULT_VERTICALPADDING_VALUE); }
void AnnotationFrameStyle::SetVerticalPadding(double value) { setRealValue(m_data, AnnotationFrameStyleProperty::VerticalPadding, DEFAULT_VERTICALPADDING_VALUE, value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameStyle::SetPadding(double value)
    {
    SetHorizontalPadding(value);
    SetVerticalPadding(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr AnnotationFrameStyle::CreateEffectiveStyle(AnnotationFrameStylePropertyBagCR overrides) const
    {
    AnnotationFrameStylePtr clone = Clone();
    clone->m_name.clear();
    clone->m_description.clear();
    clone->m_id.Invalidate();

    clone->m_data.MergeWith(overrides);

    return clone;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const uint32_t CURRENT_MAJOR_VERSION = 1;
static const uint32_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendIntegerSetter
(
FB::AnnotationFrameStyleSetters& setters,
AnnotationFrameStylePropertyBagCR data,
AnnotationFrameStyleProperty tsProp,
decltype(declval<FB::AnnotationFrameStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationFrameStyleSetter>().integerValue()) defaultValue
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetIntegerProperty(tsProp);
    if (value == defaultValue)
        return;

    setters.push_back(FB::AnnotationFrameStyleSetter(fbProp, value, 0.0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendRealSetter
(
FB::AnnotationFrameStyleSetters& setters,
AnnotationFrameStylePropertyBagCR data,
AnnotationFrameStyleProperty tsProp,
decltype(declval<FB::AnnotationFrameStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationFrameStyleSetter>().realValue()) defaultValue
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetRealProperty(tsProp);
    if (value == defaultValue)
        return;

    setters.push_back(FB::AnnotationFrameStyleSetter(fbProp, 0, value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFrameStylePersistence::EncodeAsFlatBuf(FB::AnnotationFrameStyleSetters& setters, AnnotationFrameStylePropertyBagCR data)
    {
    appendRealSetter(setters, data, AnnotationFrameStyleProperty::CloudBulgeFactor, FB::AnnotationFrameStyleProperty_CloudBulgeFactor, DEFAULT_CLOUDBULGEFACTOR_VALUE);
    appendRealSetter(setters, data, AnnotationFrameStyleProperty::CloudDiameterFactor, FB::AnnotationFrameStyleProperty_CloudDiameterFactor, DEFAULT_CLOUDDIAMETERFACTOR_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::FillColorId, FB::AnnotationFrameStyleProperty_FillColorId, DEFAULT_FILLCOLORID_VALUE);
    appendRealSetter(setters, data, AnnotationFrameStyleProperty::FillTransparency, FB::AnnotationFrameStyleProperty_FillTransparency, DEFAULT_FILLTRANSPARENCY_VALUE);
    appendRealSetter(setters, data, AnnotationFrameStyleProperty::HorizontalPadding, FB::AnnotationFrameStyleProperty_HorizontalPadding, DEFAULT_HORIZONTALPADDING_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::IsFillEnabled, FB::AnnotationFrameStyleProperty_IsFillEnabled, DEFAULT_ISFILLENABLED_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::IsStrokeCloud, FB::AnnotationFrameStyleProperty_IsStrokeCloud, DEFAULT_ISSTROKECLOUD_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::IsStrokeEnabled, FB::AnnotationFrameStyleProperty_IsStrokeEnabled, DEFAULT_ISSTROKEENABLED_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::StrokeColorId, FB::AnnotationFrameStyleProperty_StrokeColorId, DEFAULT_STROKECOLORID_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::StrokeStyle, FB::AnnotationFrameStyleProperty_StrokeStyle, DEFAULT_STROKESTYLE_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::StrokeWeight, FB::AnnotationFrameStyleProperty_StrokeWeight, DEFAULT_STROKEWEIGHT_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::Type, FB::AnnotationFrameStyleProperty_Type, DEFAULT_TYPE_VALUE);
    appendRealSetter(setters, data, AnnotationFrameStyleProperty::VerticalPadding, FB::AnnotationFrameStyleProperty_VerticalPadding, DEFAULT_VERTICALPADDING_VALUE);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFrameStylePersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, AnnotationFrameStyleCR style, FlatBufEncodeOptions options)
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

    FB::AnnotationFrameStyleSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data), ERROR);

    FB::AnnotationFrameStyleSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);

    //.............................................................................................
    FB::AnnotationFrameStyleBuilder fbStyle(encoder);
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
BentleyStatus AnnotationFrameStylePersistence::DecodeFromFlatBuf(AnnotationFrameStylePropertyBagR data, FB::AnnotationFrameStyleSetterVector const& setters)
    {
    for (auto const& setter : setters)
        {
        switch (setter.key())
            {
            case FB::AnnotationFrameStyleProperty_CloudBulgeFactor: data.SetRealProperty(AnnotationFrameStyleProperty::CloudBulgeFactor, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_CloudDiameterFactor: data.SetRealProperty(AnnotationFrameStyleProperty::CloudDiameterFactor, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_FillColorId: data.SetIntegerProperty(AnnotationFrameStyleProperty::FillColorId, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_FillTransparency: data.SetRealProperty(AnnotationFrameStyleProperty::FillTransparency, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_HorizontalPadding: data.SetRealProperty(AnnotationFrameStyleProperty::HorizontalPadding, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_IsFillEnabled: data.SetIntegerProperty(AnnotationFrameStyleProperty::IsFillEnabled, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_IsStrokeCloud: data.SetIntegerProperty(AnnotationFrameStyleProperty::IsStrokeCloud, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_IsStrokeEnabled: data.SetIntegerProperty(AnnotationFrameStyleProperty::IsStrokeEnabled, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_StrokeColorId: data.SetIntegerProperty(AnnotationFrameStyleProperty::StrokeColorId, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_StrokeStyle: data.SetIntegerProperty(AnnotationFrameStyleProperty::StrokeStyle, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_StrokeWeight: data.SetIntegerProperty(AnnotationFrameStyleProperty::StrokeWeight, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_Type: data.SetIntegerProperty(AnnotationFrameStyleProperty::Type, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_VerticalPadding: data.SetRealProperty(AnnotationFrameStyleProperty::VerticalPadding, setter.realValue()); break;
            }
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFrameStylePersistence::DecodeFromFlatBuf(AnnotationFrameStyleR style, ByteCP buffer, size_t numBytes)
    {
    style.Reset();
    
    auto fbStyle = GetRoot<FB::AnnotationFrameStyle>(buffer);

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
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr DgnAnnotationFrameStyles::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Name,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    AnnotationFrameStylePtr style = AnnotationFrameStyle::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == AnnotationFrameStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);

    style->SetDescription(query.GetValueText(1));
    style->SetId(id);
    style->SetName(query.GetValueText(0));

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr DgnAnnotationFrameStyles::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Id,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    AnnotationFrameStylePtr style = AnnotationFrameStyle::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == AnnotationFrameStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);
    
    style->SetDescription(query.GetValueText(1));
    style->SetId(query.GetValueId<DgnStyleId>(0));
    style->SetName(name);

    return style;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
DgnStyleId DgnAnnotationFrameStyles::Iterator::Entry::GetId() const { Verify(); return m_sql->GetValueId<DgnStyleId>(0); }
Utf8CP DgnAnnotationFrameStyles::Iterator::Entry::GetName() const { Verify(); return m_sql->GetValueText(1); }
Utf8CP DgnAnnotationFrameStyles::Iterator::Entry::GetDescription() const { Verify(); return m_sql->GetValueText(2); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
DgnAnnotationFrameStyles::Iterator::const_iterator DgnAnnotationFrameStyles::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,Name,Descr FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame, true);
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
size_t DgnAnnotationFrameStyles::Iterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString("SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame, true);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    m_params.Bind(query);

    POSTCONDITION(BE_SQLITE_ROW == query.Step(), 0);

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnAnnotationFrameStyles::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return false;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnAnnotationFrameStyles::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationFrameStyles::Insert(AnnotationFrameStyleR style)
    {
    // Don't assert to ensure an invalid ID.
    // Consider the case of cloning a style object, modifying, and then inserting it as a new style. The Clone keeps the ID, and I don't think it's worth having an overload of Clone to expose this detail.
    
    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationFrameStylePersistence::EncodeAsFlatBuf(data, style, AnnotationFrameStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyleId nextId;
    PRECONDITION(BE_SQLITE_OK == m_dgndb.GetNextRepositoryBasedId(nextId, DGN_TABLE(DGN_CLASSNAME_Style), "Id"), ERROR);
    
    Statement insert;
    insert.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Style) " (Id,Type,Name,Descr,Data) VALUES (?," DGN_STYLE_TYPE_AnnotationFrame ",?,?,?)");
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
BentleyStatus DgnAnnotationFrameStyles::Update(AnnotationFrameStyleCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    
    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationFrameStylePersistence::EncodeAsFlatBuf(data, style, AnnotationFrameStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    Statement update;
    update.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Style) " SET Name=?,Descr=?,Data=? WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame " AND Id=?");
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
BentleyStatus DgnAnnotationFrameStyles::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);

    Statement del;
    del.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationFrame " AND Id=?");
    del.BindId(1, id);

    POSTCONDITION(BE_SQLITE_DONE == del.Step(), ERROR);

    return SUCCESS;
    }
