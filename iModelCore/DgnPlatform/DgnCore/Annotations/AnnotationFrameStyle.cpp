//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationFrameStyle.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
AnnotationFrameStylePtr AnnotationFrameStyle::Create(DgnProjectR project) { return new AnnotationFrameStyle(project); }
AnnotationFrameStyle::AnnotationFrameStyle(DgnProjectR project) :
    T_Super()
    {
    m_project = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr AnnotationFrameStyle::Clone() const { return new AnnotationFrameStyle(*this); }
AnnotationFrameStyle::AnnotationFrameStyle(AnnotationFrameStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationFrameStyleR AnnotationFrameStyle::operator=(AnnotationFrameStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationFrameStyle::CopyFrom(AnnotationFrameStyleCR rhs)
    {
    m_project = rhs.m_project;
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
DgnProjectR AnnotationFrameStyle::GetDgnProjectR() const { return *m_project; }
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
UInt32 AnnotationFrameStyle::GetFillColorId() const { return (UInt32)getIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorId, DEFAULT_FILLCOLORID_VALUE); }
void AnnotationFrameStyle::SetFillColorId(UInt32 value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorId, DEFAULT_FILLCOLORID_VALUE, value); }

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
UInt32 AnnotationFrameStyle::GetStrokeColorId() const { return (UInt32)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorId, DEFAULT_STROKECOLORID_VALUE); }
void AnnotationFrameStyle::SetStrokeColorId(UInt32 value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorId, DEFAULT_STROKECOLORID_VALUE, value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_STROKESTYLE_VALUE = 0;
Int32 AnnotationFrameStyle::GetStrokeStyle() const { return (Int32)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeStyle, DEFAULT_STROKESTYLE_VALUE); }
void AnnotationFrameStyle::SetStrokeStyle(Int32 value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeStyle, DEFAULT_STROKESTYLE_VALUE, (UInt32)value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_STROKEWEIGHT_VALUE = 0;
UInt32 AnnotationFrameStyle::GetStrokeWeight() const { return (UInt32)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeWeight, DEFAULT_STROKEWEIGHT_VALUE); }
void AnnotationFrameStyle::SetStrokeWeight(UInt32 value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeWeight, DEFAULT_STROKEWEIGHT_VALUE, value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_TYPE_VALUE = (AnnotationFrameStylePropertyBag::T_Integer)AnnotationFrameType::InvisibleBox;
AnnotationFrameType AnnotationFrameStyle::GetType() const { return (AnnotationFrameType)getIntegerValue(m_data, AnnotationFrameStyleProperty::Type, DEFAULT_TYPE_VALUE); }
void AnnotationFrameStyle::SetType(AnnotationFrameType value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::Type, DEFAULT_TYPE_VALUE, (UInt32)value); }

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

static const UInt32 CURRENT_MAJOR_VERSION = 1;
static const UInt32 CURRENT_MINOR_VERSION = 0;

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
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr DgnAnnotationFrameStyles::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), NULL);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::AnnotationFrame, id);
    if (!styleRow.GetId().IsValid())
        return NULL;

    AnnotationFrameStylePtr style = AnnotationFrameStyle::Create(m_project);
    
    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Therefore, do this before setting the fields from the table.
    POSTCONDITION(SUCCESS == AnnotationFrameStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)styleRow.GetData(), styleRow.GetDataSize()), NULL);
    
    style->SetDescription(styleRow.GetDescription());
    style->SetId(id);
    style->SetName(styleRow.GetName());

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr DgnAnnotationFrameStyles::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), NULL);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::AnnotationFrame, name);
    if (!id.IsValid())
        return NULL;

    return QueryById(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
bool DgnAnnotationFrameStyles::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::AnnotationFrame, id);
    
    return styleRow.GetId().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
bool DgnAnnotationFrameStyles::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::AnnotationFrame, name);
    
    return id.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr DgnAnnotationFrameStyles::Insert(AnnotationFrameStyleCR style)
    {
    PRECONDITION(!style.GetName().empty(), NULL);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, NULL);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == AnnotationFrameStylePersistence::EncodeAsFlatBuf(styleData, style, AnnotationFrameStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), NULL);

    DgnStyles::Style styleRow(DgnStyleId(), DgnStyleType::AnnotationFrame, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().InsertStyle(styleRow)))
        return NULL;

    return QueryById(styleRow.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationFrameStyles::Update(AnnotationFrameStyleCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    PRECONDITION(!style.GetName().empty(), ERROR);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, ERROR);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == AnnotationFrameStylePersistence::EncodeAsFlatBuf(styleData, style, AnnotationFrameStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyles::Style styleRow(style.GetId(), DgnStyleType::AnnotationFrame, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().UpdateStyle(styleRow)))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationFrameStyles::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);
    
    POSTCONDITION(BE_SQLITE_DONE == m_project.Styles().DeleteStyle(DgnStyleType::AnnotationFrame, id), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
DgnStyles::Iterator DgnAnnotationFrameStyles::MakeIterator(DgnStyleSort sortOrder) const
    {
    Utf8String queryModifierClauses;
    queryModifierClauses.Sprintf("WHERE Type=%d", DgnStyleType::AnnotationFrame);

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
