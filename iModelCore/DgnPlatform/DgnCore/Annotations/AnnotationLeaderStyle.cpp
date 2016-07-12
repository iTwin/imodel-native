//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationLeaderStyle.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGN
using namespace flatbuffers;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationLeaderStylePropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((AnnotationLeaderStyleProperty)key)
        {
        case AnnotationLeaderStyleProperty::LineColorType:
        case AnnotationLeaderStyleProperty::LineColorValue:
        case AnnotationLeaderStyleProperty::LineType:
        case AnnotationLeaderStyleProperty::LineWeight:
        case AnnotationLeaderStyleProperty::TerminatorColorType:
        case AnnotationLeaderStyleProperty::TerminatorColorValue:
        case AnnotationLeaderStyleProperty::TerminatorType:
        case AnnotationLeaderStyleProperty::TerminatorWeight:
            return true;

        default:
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationLeaderStylePropertyBag::_IsRealProperty(T_Key key) const
    {
    switch ((AnnotationLeaderStyleProperty)key)
        {
        case AnnotationLeaderStyleProperty::TerminatorScaleFactor:
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
HANDLER_DEFINE_MEMBERS(AnnotationLeaderStyleHandler);

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void AnnotationLeaderStyleHandler::_TEMPORARY_GetHandlingCustomAttributes(ECSqlClassParams::HandlingCustomAttributes& params) // *** WIP_AUTO_HANDLED_PROPERTIES
    {
    T_Super::_TEMPORARY_GetHandlingCustomAttributes(params);
    params.Add(PROP_Data);
    params.Add(PROP_Description);
    }
}
END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationLeaderStyle::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& select, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(select, params);
    if (DgnDbStatus::Success != status)
        return status;

    int dataSize = 0;
    ByteCP data = static_cast<ByteCP>(select.GetValueBinary(params.GetSelectIndex(PROP_Data), &dataSize));
    if (SUCCESS != AnnotationLeaderStylePersistence::DecodeFromFlatBuf(*this, data, static_cast<size_t>(dataSize)))
        return DgnDbStatus::BadArg;

    m_description.AssignOrClear(select.GetValueText(params.GetSelectIndex(PROP_Description)));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
static DgnDbStatus bindParams(BeSQLite::EC::ECSqlStatement& stmt, AnnotationLeaderStyleCR style)
    {
    bvector<Byte> data;
    if (SUCCESS != AnnotationLeaderStylePersistence::EncodeAsFlatBuf(data, style, AnnotationLeaderStylePersistence::FlatBufEncodeOptions::Default))
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
DgnDbStatus AnnotationLeaderStyle::_BindInsertParams(BeSQLite::EC::ECSqlStatement& insert)
    {
    DgnDbStatus status = T_Super::_BindInsertParams(insert);
    if (DgnDbStatus::Success != status)
        return status;

    return bindParams(insert, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationLeaderStyle::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& update)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(update);
    if (DgnDbStatus::Success != status)
        return status;

    return bindParams(update, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void AnnotationLeaderStyle::_CopyFrom(DgnElementCR src)
    {
    T_Super::_CopyFrom(src);

    AnnotationLeaderStyleCP rhs = dynamic_cast<AnnotationLeaderStyleCP>(&src);
    if (nullptr == rhs)
        return;

    m_description = rhs->m_description;
    m_data = rhs->m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static AnnotationTextStylePropertyBag::T_Integer getIntegerValue(AnnotationLeaderStylePropertyBagCR data, AnnotationLeaderStyleProperty key, AnnotationTextStylePropertyBag::T_Integer defaultValue)
    {
    if (data.HasProperty(key))
        return data.GetIntegerProperty(key);

    return defaultValue;
    }
static void setIntegerValue(AnnotationLeaderStylePropertyBagR data, AnnotationLeaderStyleProperty key, AnnotationTextStylePropertyBag::T_Integer defaultValue, AnnotationTextStylePropertyBag::T_Integer value)
    {
    if (value == defaultValue)
        data.ClearProperty(key);
    else
        data.SetIntegerProperty(key, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static AnnotationTextStylePropertyBag::T_Real getRealValue(AnnotationLeaderStylePropertyBagCR data, AnnotationLeaderStyleProperty key, AnnotationTextStylePropertyBag::T_Real defaultValue)
    {
    if (data.HasProperty(key))
        return data.GetRealProperty(key);

    return defaultValue;
    }
static void setRealValue(AnnotationLeaderStylePropertyBagR data, AnnotationLeaderStyleProperty key, AnnotationTextStylePropertyBag::T_Real defaultValue, AnnotationTextStylePropertyBag::T_Real value)
    {
    if (value == defaultValue)
        data.ClearProperty(key);
    else
        data.SetRealProperty(key, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_LINECOLOR_TYPE_VALUE = (AnnotationLeaderStylePropertyBag::T_Integer)AnnotationColorType::ByCategory;
AnnotationColorType AnnotationLeaderStyle::GetLineColorType() const { return (AnnotationColorType)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColorType, DEFAULT_LINECOLOR_TYPE_VALUE); }
void AnnotationLeaderStyle::SetLineColorType(AnnotationColorType value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColorType, DEFAULT_LINECOLOR_TYPE_VALUE, (AnnotationLeaderStylePropertyBag::T_Integer)value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_LINECOLOR_VALUE_VALUE = 0;
ColorDef AnnotationLeaderStyle::GetLineColorValue() const { return ColorDef((uint32_t)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColorValue, DEFAULT_LINECOLOR_VALUE_VALUE)); }
void AnnotationLeaderStyle::SetLineColorValue(ColorDef value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColorValue, DEFAULT_LINECOLOR_VALUE_VALUE, value.GetValue()); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_LINETYPE_VALUE = (AnnotationLeaderStylePropertyBag::T_Integer)AnnotationLeaderLineType::None;
AnnotationLeaderLineType AnnotationLeaderStyle::GetLineType() const { return (AnnotationLeaderLineType)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineType, DEFAULT_LINETYPE_VALUE); }
void AnnotationLeaderStyle::SetLineType(AnnotationLeaderLineType value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineType, DEFAULT_LINETYPE_VALUE, (uint32_t)value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_LINEWEIGHT_VALUE = 0;
uint32_t AnnotationLeaderStyle::GetLineWeight() const { return (uint32_t)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineWeight, DEFAULT_LINEWEIGHT_VALUE); }
void AnnotationLeaderStyle::SetLineWeight(uint32_t value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineWeight, DEFAULT_LINEWEIGHT_VALUE, value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_TERMINATORCOLOR_TYPE_VALUE = (AnnotationLeaderStylePropertyBag::T_Integer)AnnotationColorType::ByCategory;
AnnotationColorType AnnotationLeaderStyle::GetTerminatorColorType() const { return (AnnotationColorType)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColorType, DEFAULT_TERMINATORCOLOR_TYPE_VALUE); }
void AnnotationLeaderStyle::SetTerminatorColorType(AnnotationColorType value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColorType, DEFAULT_TERMINATORCOLOR_TYPE_VALUE, (AnnotationLeaderStylePropertyBag::T_Integer)value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_TERMINATORCOLOR_VALUE_VALUE = 0;
ColorDef AnnotationLeaderStyle::GetTerminatorColorValue() const { return ColorDef((uint32_t)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColorValue, DEFAULT_TERMINATORCOLOR_VALUE_VALUE)); }
void AnnotationLeaderStyle::SetTerminatorColorValue(ColorDef value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColorValue, DEFAULT_TERMINATORCOLOR_VALUE_VALUE, value.GetValue()); }

static const AnnotationLeaderStylePropertyBag::T_Real DEFAULT_TERMINATORSCALEFACTOR_VALUE = 1.0;
double AnnotationLeaderStyle::GetTerminatorScaleFactor() const { return getRealValue(m_data, AnnotationLeaderStyleProperty::TerminatorScaleFactor, DEFAULT_TERMINATORSCALEFACTOR_VALUE); }
void AnnotationLeaderStyle::SetTerminatorScaleFactor(double value) { setRealValue(m_data, AnnotationLeaderStyleProperty::TerminatorScaleFactor, DEFAULT_TERMINATORSCALEFACTOR_VALUE, value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_TERMINATORTYPE_VALUE = (AnnotationLeaderStylePropertyBag::T_Integer)AnnotationLeaderTerminatorType::None;
AnnotationLeaderTerminatorType AnnotationLeaderStyle::GetTerminatorType() const { return (AnnotationLeaderTerminatorType)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorType, DEFAULT_TERMINATORTYPE_VALUE); }
void AnnotationLeaderStyle::SetTerminatorType(AnnotationLeaderTerminatorType value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorType, DEFAULT_TERMINATORTYPE_VALUE, (uint32_t)value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_TERMINATORWEIGHT_VALUE = 0;
uint32_t AnnotationLeaderStyle::GetTerminatorWeight() const { return (uint32_t)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorWeight, DEFAULT_TERMINATORWEIGHT_VALUE); }
void AnnotationLeaderStyle::SetTerminatorWeight(uint32_t value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorWeight, DEFAULT_TERMINATORWEIGHT_VALUE, value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr AnnotationLeaderStyle::CreateEffectiveStyle(AnnotationLeaderStylePropertyBagCR overrides) const
    {
    AnnotationLeaderStylePtr copy = CreateCopy();
    copy->InvalidateElementId();
    copy->InvalidateCode();
    copy->m_description.clear();
    copy->m_data.MergeWith(overrides);

    return copy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
size_t AnnotationLeaderStyle::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr select = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationLeaderStyle));
    if (!select.IsValid())
        return 0;

    if (BE_SQLITE_ROW != select->Step())
        return 0;

    return static_cast<size_t>(select->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
AnnotationLeaderStyle::Iterator AnnotationLeaderStyle::MakeIterator(DgnDbR db)
    {
    Iterator iter;
    iter.Prepare(db, "SELECT ECInstanceId, Code.[Value], Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_AnnotationLeaderStyle), 0);

    return iter;
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
FB::AnnotationLeaderStyleSetters& setters,
AnnotationLeaderStylePropertyBagCR data,
AnnotationLeaderStyleProperty tsProp,
decltype(declval<FB::AnnotationLeaderStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationLeaderStyleSetter>().integerValue()) defaultValue,
bool writeIfDefault
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetIntegerProperty(tsProp);
    if (!writeIfDefault && (value == defaultValue))
        return;

    setters.push_back(FB::AnnotationLeaderStyleSetter(fbProp, value, 0.0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void appendRealSetter
(
FB::AnnotationLeaderStyleSetters& setters,
AnnotationLeaderStylePropertyBagCR data,
AnnotationLeaderStyleProperty tsProp,
decltype(declval<FB::AnnotationLeaderStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationLeaderStyleSetter>().realValue()) defaultValue,
bool writeIfDefault
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetRealProperty(tsProp);
    if (!writeIfDefault && (value == defaultValue))
        return;

    setters.push_back(FB::AnnotationLeaderStyleSetter(fbProp, 0, value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderStylePersistence::EncodeAsFlatBuf(FB::AnnotationLeaderStyleSetters& setters, AnnotationLeaderStylePropertyBagCR data) { return EncodeAsFlatBuf(setters, data, FlatBufEncodeOptions::Default); }
BentleyStatus AnnotationLeaderStylePersistence::EncodeAsFlatBuf(FB::AnnotationLeaderStyleSetters& setters, AnnotationLeaderStylePropertyBagCR data, FlatBufEncodeOptions options)
    {
    bool writeIfDefault = isEnumFlagSet(FlatBufEncodeOptions::SettersAreOverrides, options);

    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineColorType, FB::AnnotationLeaderStyleProperty_LineColorType, DEFAULT_LINECOLOR_TYPE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineColorValue, FB::AnnotationLeaderStyleProperty_LineColorValue, DEFAULT_LINECOLOR_VALUE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineType, FB::AnnotationLeaderStyleProperty_LineType, DEFAULT_LINETYPE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineWeight, FB::AnnotationLeaderStyleProperty_LineWeight, DEFAULT_LINEWEIGHT_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorColorType, FB::AnnotationLeaderStyleProperty_TerminatorColorType, DEFAULT_TERMINATORCOLOR_TYPE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorColorValue, FB::AnnotationLeaderStyleProperty_TerminatorColorValue, DEFAULT_TERMINATORCOLOR_VALUE_VALUE, writeIfDefault);
    appendRealSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorScaleFactor, FB::AnnotationLeaderStyleProperty_TerminatorScaleFactor, DEFAULT_TERMINATORSCALEFACTOR_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorType, FB::AnnotationLeaderStyleProperty_TerminatorType, DEFAULT_TERMINATORTYPE_VALUE, writeIfDefault);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorWeight, FB::AnnotationLeaderStyleProperty_TerminatorWeight, DEFAULT_TERMINATORWEIGHT_VALUE, writeIfDefault);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderStylePersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, AnnotationLeaderStyleCR style, FlatBufEncodeOptions options)
    {
    FlatBufferBuilder encoder;
    
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::AnnotationLeaderStyleSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data, options), ERROR);

    FB::AnnotationLeaderStyleSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);
    
    //.............................................................................................
    FB::AnnotationLeaderStyleBuilder fbStyle(encoder);
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
BentleyStatus AnnotationLeaderStylePersistence::DecodeFromFlatBuf(AnnotationLeaderStylePropertyBagR data, FB::AnnotationLeaderStyleSetterVector const& setters)
    {
    for (auto const& setter : setters)
        {
        switch (setter.key())
            {
            case FB::AnnotationLeaderStyleProperty_LineColorType: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineColorType, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineColorValue: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineColorValue, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineType: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineType, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineWeight: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineWeight, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorColorType: data.SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorColorType, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorColorValue: data.SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorColorValue, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorScaleFactor: data.SetRealProperty(AnnotationLeaderStyleProperty::TerminatorScaleFactor, setter.realValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorType: data.SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorType, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorWeight: data.SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorWeight, setter.integerValue()); break;
            }
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderStylePersistence::DecodeFromFlatBuf(AnnotationLeaderStyleR style, ByteCP buffer, size_t numBytes)
    {
    style.m_data.ClearAllProperties();
    
    auto fbStyle = GetRoot<FB::AnnotationLeaderStyle>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }
