//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotationSeed.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
bool TextAnnotationSeedPropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((TextAnnotationSeedProperty)key)
        {
        case TextAnnotationSeedProperty::FrameStyleId:
        case TextAnnotationSeedProperty::LeaderStyleId:
        case TextAnnotationSeedProperty::TextStyleId:
            return true;

        default:
            return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
bool TextAnnotationSeedPropertyBag::_IsRealProperty(T_Key key) const
    {
    return false;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

#define PROP_Data "Data"
#define PROP_Description "Descr"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(TextAnnotationSeedHandler);

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void TextAnnotationSeedHandler::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(PROP_Data);
    params.Add(PROP_Description);
    }
}
END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus TextAnnotationSeed::_ExtractSelectParams(BeSQLite::EC::ECSqlStatement& select, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ExtractSelectParams(select, params);
    if (DgnDbStatus::Success != status)
        return status;

    int dataSize = 0;
    ByteCP data = static_cast<ByteCP>(select.GetValueBinary(params.GetSelectIndex(PROP_Data), &dataSize));
    if (SUCCESS != TextAnnotationSeedPersistence::DecodeFromFlatBuf(*this, data, static_cast<size_t>(dataSize)))
        return DgnDbStatus::BadArg;

    m_description.AssignOrClear(select.GetValueText(params.GetSelectIndex(PROP_Description)));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
static DgnDbStatus bindParams(BeSQLite::EC::ECSqlStatement& stmt, TextAnnotationSeedCR style)
    {
    bvector<Byte> data;
    if (SUCCESS != TextAnnotationSeedPersistence::EncodeAsFlatBuf(data, style))
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
DgnDbStatus TextAnnotationSeed::_BindInsertParams(BeSQLite::EC::ECSqlStatement& insert)
    {
    DgnDbStatus status = T_Super::_BindInsertParams(insert);
    if (DgnDbStatus::Success != status)
        return status;

    return bindParams(insert, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus TextAnnotationSeed::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& update)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(update);
    if (DgnDbStatus::Success != status)
        return status;

    return bindParams(update, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void TextAnnotationSeed::_CopyFrom(DgnElementCR src)
    {
    T_Super::_CopyFrom(src);

    TextAnnotationSeedCP rhs = dynamic_cast<TextAnnotationSeedCP>(&src);
    if (nullptr == rhs)
        return;

    m_description = rhs->m_description;
    m_data = rhs->m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
static TextAnnotationSeedPropertyBag::T_Integer getIntegerValue(TextAnnotationSeedPropertyBagCR data, TextAnnotationSeedProperty key, TextAnnotationSeedPropertyBag::T_Integer defaultValue)
    {
    if (data.HasProperty(key))
        return data.GetIntegerProperty(key);

    return defaultValue;
    }
static void setIntegerValue(TextAnnotationSeedPropertyBagR data, TextAnnotationSeedProperty key, TextAnnotationSeedPropertyBag::T_Integer defaultValue, TextAnnotationSeedPropertyBag::T_Integer value)
    {
    if (value == defaultValue)
        data.ClearProperty(key);
    else
        data.SetIntegerProperty(key, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
// static TextAnnotationSeedPropertyBag::T_Real getRealValue(TextAnnotationSeedPropertyBagCR data, TextAnnotationSeedProperty key, TextAnnotationSeedPropertyBag::T_Real defaultValue)
//     {
//     if (data.HasProperty(key))
//         return data.GetRealProperty(key);
//     
//     return defaultValue;
//     }
// static void setRealValue(TextAnnotationSeedPropertyBagR data, TextAnnotationSeedProperty key, TextAnnotationSeedPropertyBag::T_Real defaultValue, TextAnnotationSeedPropertyBag::T_Real value)
//     {
//     if (value == defaultValue)
//         data.ClearProperty(key);
//     else
//         data.SetRealProperty(key, value);
//     }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_FRAMESTYLEID_VALUE = 0;
DgnElementId TextAnnotationSeed::GetFrameStyleId() const { return DgnElementId((uint64_t)getIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE)); }
void TextAnnotationSeed::SetFrameStyleId(DgnElementId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_LEADERSTYLEID_VALUE = 0;
DgnElementId TextAnnotationSeed::GetLeaderStyleId() const { return DgnElementId((uint64_t)getIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE)); }
void TextAnnotationSeed::SetLeaderStyleId(DgnElementId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_TEXTSTYLEID_VALUE = 0;
DgnElementId TextAnnotationSeed::GetTextStyleId() const { return DgnElementId((uint64_t)getIntegerValue(m_data, TextAnnotationSeedProperty::TextStyleId, DEFAULT_TEXTSTYLEID_VALUE)); }
void TextAnnotationSeed::SetTextStyleId(DgnElementId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::TextStyleId, DEFAULT_TEXTSTYLEID_VALUE, value.GetValue()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr TextAnnotationSeed::CreateEffectiveSeed(TextAnnotationSeedPropertyBagCR overrides) const
    {
    TextAnnotationSeedPtr copy = CreateCopy();
    copy->InvalidateElementId();
    copy->InvalidateCode();
    copy->m_description.clear();
    copy->m_data.MergeWith(overrides);

    return copy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
size_t TextAnnotationSeed::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr select = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationSeed));
    if (!select.IsValid())
        return 0;

    if (BE_SQLITE_ROW != select->Step())
        return 0;

    return static_cast<size_t>(select->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
TextAnnotationSeed::Iterator TextAnnotationSeed::MakeIterator(DgnDbR db)
    {
    Iterator iter;
    iter.Prepare(db, "SELECT ECInstanceId, Code, Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationSeed), 0);

    return iter;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const uint32_t CURRENT_MAJOR_VERSION = 1;
static const uint32_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
static void appendIntegerSetter
(
FB::TextAnnotationSeedSetters& setters,
TextAnnotationSeedPropertyBagCR data,
TextAnnotationSeedProperty tsProp,
decltype(declval<FB::TextAnnotationSeedSetter>().key()) fbProp,
decltype(declval<FB::TextAnnotationSeedSetter>().integerValue()) defaultValue
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetIntegerProperty(tsProp);
    if (value == defaultValue)
        return;

    setters.push_back(FB::TextAnnotationSeedSetter(fbProp, value, 0.0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
// static void appendRealSetter
// (
// FB::TextAnnotationSeedSetters& setters,
// TextAnnotationSeedPropertyBagCR data,
// TextAnnotationSeedProperty tsProp,
// decltype(declval<FB::TextAnnotationSeedSetter>().key()) fbProp,
// decltype(declval<FB::TextAnnotationSeedSetter>().realValue()) defaultValue
// )
//     {
//     if (!data.HasProperty(tsProp))
//         return;

//     auto value = data.GetRealProperty(tsProp);
//     if (value == defaultValue)
//         return;

//     setters.push_back(FB::TextAnnotationSeedSetter(fbProp, 0, value));
//     }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationSeedPersistence::EncodeAsFlatBuf(FB::TextAnnotationSeedSetters& setters, TextAnnotationSeedPropertyBagCR data)
    {
    appendIntegerSetter(setters, data, TextAnnotationSeedProperty::FrameStyleId, FB::TextAnnotationSeedProperty_FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE);
    appendIntegerSetter(setters, data, TextAnnotationSeedProperty::LeaderStyleId, FB::TextAnnotationSeedProperty_LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE);
    appendIntegerSetter(setters, data, TextAnnotationSeedProperty::TextStyleId, FB::TextAnnotationSeedProperty_TextStyleId, DEFAULT_TEXTSTYLEID_VALUE);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationSeedPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, TextAnnotationSeedCR style)
    {
    FlatBufferBuilder encoder;
    
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::TextAnnotationSeedSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data), ERROR);

    FB::TextAnnotationSeedSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);

    //.............................................................................................
    FB::TextAnnotationSeedBuilder fbStyle(encoder);
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
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationSeedPersistence::DecodeFromFlatBuf(TextAnnotationSeedPropertyBagR data, FB::TextAnnotationSeedSetterVector const& setters)
    {
    for (auto const& setter : setters)
        {
        switch (setter.key())
            {
            case FB::TextAnnotationSeedProperty_FrameStyleId: data.SetIntegerProperty(TextAnnotationSeedProperty::FrameStyleId, setter.integerValue()); break;
            case FB::TextAnnotationSeedProperty_LeaderStyleId: data.SetIntegerProperty(TextAnnotationSeedProperty::LeaderStyleId, setter.integerValue()); break;
            case FB::TextAnnotationSeedProperty_TextStyleId: data.SetIntegerProperty(TextAnnotationSeedProperty::TextStyleId, setter.integerValue()); break;
            }
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationSeedPersistence::DecodeFromFlatBuf(TextAnnotationSeedR style, ByteCP buffer, size_t numBytes)
    {
    style.m_data.ClearAllProperties();
    
    auto fbStyle = GetRoot<FB::TextAnnotationSeed>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }
