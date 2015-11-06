//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationFrameStyle.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
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
        case AnnotationFrameStyleProperty::FillColorType:
        case AnnotationFrameStyleProperty::FillColorValue:
        case AnnotationFrameStyleProperty::IsFillEnabled:
        case AnnotationFrameStyleProperty::IsStrokeCloud:
        case AnnotationFrameStyleProperty::IsStrokeEnabled:
        case AnnotationFrameStyleProperty::StrokeColorType:
        case AnnotationFrameStyleProperty::StrokeColorValue:
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

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

#define PROP_Data "Data"
#define PROP_Descr "Descr"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(AnnotationFrameStyleHandler);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationFrameStyleHandler::_GetClassParams(ECSqlClassParams& params)
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
AnnotationFrameStyle::CreateParams::CreateParams(DgnDbR db, Utf8StringCR name, AnnotationFrameStylePropertyBagCR data, Utf8StringCR descr)
    : T_Super(db, QueryDgnClassId(db), CreateStyleCode(name)), m_data(data), m_descr(descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationFrameStyle::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationFrameStylePersistence::EncodeAsFlatBuf(data, *this, AnnotationFrameStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), DgnDbStatus::BadArg);

    return ECSqlStatus::Success == stmt.BindText(stmt.GetParameterIndex(PROP_Descr), m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        && ECSqlStatus::Success == stmt.BindBinary(stmt.GetParameterIndex(PROP_Data), &data[0], static_cast<int>(data.size()), IECSqlBinder::MakeCopy::Yes)
        ? DgnDbStatus::Success : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationFrameStyle::_ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        int dataSize = 0;
        Byte const* data = static_cast<Byte const*>(stmt.GetValueBinary(params.GetSelectIndex(PROP_Data), &dataSize));
        POSTCONDITION(SUCCESS == AnnotationFrameStylePersistence::DecodeFromFlatBuf(*this, data, static_cast<size_t>(dataSize)), DgnDbStatus::BadArg);

        m_descr.AssignOrClear(stmt.GetValueText(params.GetSelectIndex(PROP_Descr)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationFrameStyle::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AnnotationFrameStyle::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationFrameStyle::_CopyFrom(DgnElementCR src)
    {
    T_Super::_CopyFrom(src);
    auto other = dynamic_cast<AnnotationFrameStyleCP>(&src);
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
DgnDbStatus AnnotationFrameStyle::_OnDelete() const
    {
    return DgnDbStatus::DeletionProhibited; // purge only
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationFrameStyleId AnnotationFrameStyle::QueryStyleId(Code const& code, DgnDbR db)
    {
    return AnnotationFrameStyleId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameStyle::Reset()
    {
    InvalidateElementId();
    InvalidateCode();
    m_descr.clear();
    ResetProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationFrameStyle::ResetProperties()
    {
    m_data.ClearAllProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool AnnotationFrameStyle::ExistsById(AnnotationFrameStyleId id, DgnDbR db)
    {
    PRECONDITION(id.IsValid(), false);

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationFrameStyle) " WHERE ECInstanceId=? LIMIT 1");
    if (stmt.IsValid())
        {
        stmt->BindId(1, id);
        if (BE_SQLITE_ROW == stmt->Step())
            return 0 < stmt->GetValueInt(0);
        }

    return false;
    }

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

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_FILLCOLOR_TYPE_VALUE = (AnnotationFrameStylePropertyBag::T_Integer)AnnotationColorType::ByCategory;
AnnotationColorType AnnotationFrameStyle::GetFillColorType() const { return (AnnotationColorType)getIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorType, DEFAULT_FILLCOLOR_TYPE_VALUE); }
void AnnotationFrameStyle::SetFillColorType(AnnotationColorType value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorType, DEFAULT_FILLCOLOR_TYPE_VALUE, (AnnotationFrameStylePropertyBag::T_Integer)value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_FILLCOLOR_VALUE_VALUE = 0;
ColorDef AnnotationFrameStyle::GetFillColorValue() const { return ColorDef((uint32_t)getIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorValue, DEFAULT_FILLCOLOR_VALUE_VALUE)); }
void AnnotationFrameStyle::SetFillColorValue(ColorDef value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::FillColorValue, DEFAULT_FILLCOLOR_VALUE_VALUE, (AnnotationFrameStylePropertyBag::T_Integer)value.GetValue()); }

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

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_STROKECOLOR_TYPE_VALUE = (AnnotationFrameStylePropertyBag::T_Integer)AnnotationColorType::ByCategory;
AnnotationColorType AnnotationFrameStyle::GetStrokeColorType() const { return (AnnotationColorType)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorType, DEFAULT_STROKECOLOR_TYPE_VALUE); }
void AnnotationFrameStyle::SetStrokeColorType(AnnotationColorType value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorType, DEFAULT_STROKECOLOR_TYPE_VALUE, (AnnotationFrameStylePropertyBag::T_Integer)value); }

static const AnnotationFrameStylePropertyBag::T_Integer DEFAULT_STROKECOLOR_VALUE_VALUE = 0;
ColorDef AnnotationFrameStyle::GetStrokeColorValue() const { return ColorDef((uint32_t)getIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorValue, DEFAULT_STROKECOLOR_VALUE_VALUE)); }
void AnnotationFrameStyle::SetStrokeColorValue(ColorDef value) { setIntegerValue(m_data, AnnotationFrameStyleProperty::StrokeColorValue, DEFAULT_STROKECOLOR_VALUE_VALUE, value.GetValue()); }

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
AnnotationFrameStylePtr AnnotationFrameStyle::CreateEffectiveStyle(AnnotationFrameStylePropertyBagCR overrides) const
    {
    AnnotationFrameStylePtr clone = Clone();
    clone->InvalidateCode();
    clone->m_descr.clear();
    clone->InvalidateElementId();

    clone->m_data.MergeWith(overrides);

    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AnnotationFrameStyle::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationFrameStyle));
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? static_cast<size_t>(stmt->GetValueInt(0)) : 0;
    }

#define SELECT_AnnotationFrameStyle "SELECT ECInstanceId, Code, Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationFrameStyle)
#define SELECT_ORDERED_AnnotationFrameStyle SELECT_AnnotationFrameStyle " ORDER BY Code"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationFrameStyle::Iterator AnnotationFrameStyle::MakeIterator(DgnDbR db, bool ordered)
    {
    Utf8CP ecSql = ordered? SELECT_ORDERED_AnnotationFrameStyle : SELECT_AnnotationFrameStyle;

    Iterator iter;
    iter.Prepare(db, ecSql, 0);

    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t AnnotationFrameStyle::_GetMemSize() const
    {
    return T_Super::_GetMemSize() + m_data.GetMemSize() + static_cast<uint32_t>(m_descr.length());
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
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::FillColorType, FB::AnnotationFrameStyleProperty_FillColorType, DEFAULT_FILLCOLOR_TYPE_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::FillColorValue, FB::AnnotationFrameStyleProperty_FillColorValue, DEFAULT_FILLCOLOR_VALUE_VALUE);
    appendRealSetter(setters, data, AnnotationFrameStyleProperty::FillTransparency, FB::AnnotationFrameStyleProperty_FillTransparency, DEFAULT_FILLTRANSPARENCY_VALUE);
    appendRealSetter(setters, data, AnnotationFrameStyleProperty::HorizontalPadding, FB::AnnotationFrameStyleProperty_HorizontalPadding, DEFAULT_HORIZONTALPADDING_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::IsFillEnabled, FB::AnnotationFrameStyleProperty_IsFillEnabled, DEFAULT_ISFILLENABLED_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::IsStrokeCloud, FB::AnnotationFrameStyleProperty_IsStrokeCloud, DEFAULT_ISSTROKECLOUD_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::IsStrokeEnabled, FB::AnnotationFrameStyleProperty_IsStrokeEnabled, DEFAULT_ISSTROKEENABLED_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::StrokeColorType, FB::AnnotationFrameStyleProperty_StrokeColorType, DEFAULT_STROKECOLOR_TYPE_VALUE);
    appendIntegerSetter(setters, data, AnnotationFrameStyleProperty::StrokeColorValue, FB::AnnotationFrameStyleProperty_StrokeColorValue, DEFAULT_STROKECOLOR_VALUE_VALUE);
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
    FB::AnnotationFrameStyleSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data), ERROR);

    FB::AnnotationFrameStyleSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);

    //.............................................................................................
    FB::AnnotationFrameStyleBuilder fbStyle(encoder);
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
BentleyStatus AnnotationFrameStylePersistence::DecodeFromFlatBuf(AnnotationFrameStylePropertyBagR data, FB::AnnotationFrameStyleSetterVector const& setters)
    {
    for (auto const& setter : setters)
        {
        switch (setter.key())
            {
            case FB::AnnotationFrameStyleProperty_CloudBulgeFactor: data.SetRealProperty(AnnotationFrameStyleProperty::CloudBulgeFactor, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_CloudDiameterFactor: data.SetRealProperty(AnnotationFrameStyleProperty::CloudDiameterFactor, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_FillColorType: data.SetIntegerProperty(AnnotationFrameStyleProperty::FillColorType, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_FillColorValue: data.SetIntegerProperty(AnnotationFrameStyleProperty::FillColorValue, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_FillTransparency: data.SetRealProperty(AnnotationFrameStyleProperty::FillTransparency, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_HorizontalPadding: data.SetRealProperty(AnnotationFrameStyleProperty::HorizontalPadding, setter.realValue()); break;
            case FB::AnnotationFrameStyleProperty_IsFillEnabled: data.SetIntegerProperty(AnnotationFrameStyleProperty::IsFillEnabled, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_IsStrokeCloud: data.SetIntegerProperty(AnnotationFrameStyleProperty::IsStrokeCloud, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_IsStrokeEnabled: data.SetIntegerProperty(AnnotationFrameStyleProperty::IsStrokeEnabled, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_StrokeColorType: data.SetIntegerProperty(AnnotationFrameStyleProperty::StrokeColorType, setter.integerValue()); break;
            case FB::AnnotationFrameStyleProperty_StrokeColorValue: data.SetIntegerProperty(AnnotationFrameStyleProperty::StrokeColorValue, setter.integerValue()); break;
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
    style.ResetProperties();
    
    auto fbStyle = GetRoot<FB::AnnotationFrameStyle>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }

