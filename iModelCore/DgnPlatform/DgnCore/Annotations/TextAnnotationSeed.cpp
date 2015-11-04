//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotationSeed.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGN
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
#define PROP_Descr "Descr"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(TextAnnotationSeedHandler);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TextAnnotationSeedHandler::_GetClassParams(ECSqlClassParams& params)
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
TextAnnotationSeed::CreateParams::CreateParams(DgnDbR db, Utf8StringCR name, TextAnnotationSeedPropertyBagCR data, Utf8StringCR descr)
    : T_Super(db, QueryDgnClassId(db), CreateCodeForSeed(name)), m_data(data), m_descr(descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TextAnnotationSeed::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    bvector<Byte> data;
    PRECONDITION(SUCCESS == TextAnnotationSeedPersistence::EncodeAsFlatBuf(data, *this, TextAnnotationSeedPersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), DgnDbStatus::BadArg);

    return ECSqlStatus::Success == stmt.BindText(stmt.GetParameterIndex(PROP_Descr), m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        && ECSqlStatus::Success == stmt.BindBinary(stmt.GetParameterIndex(PROP_Data), &data[0], static_cast<int>(data.size()), IECSqlBinder::MakeCopy::Yes)
        ? DgnDbStatus::Success : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TextAnnotationSeed::_ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        int dataSize = 0;
        Byte const* data = static_cast<Byte const*>(stmt.GetValueBinary(params.GetSelectIndex(PROP_Data), &dataSize));
        POSTCONDITION(SUCCESS == TextAnnotationSeedPersistence::DecodeFromFlatBuf(*this, data, static_cast<size_t>(dataSize)), DgnDbStatus::BadArg);

        m_descr.AssignOrClear(stmt.GetValueText(params.GetSelectIndex(PROP_Descr)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TextAnnotationSeed::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TextAnnotationSeed::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TextAnnotationSeed::_CopyFrom(DgnElementCR src)
    {
    T_Super::_CopyFrom(src);
    auto other = dynamic_cast<TextAnnotationSeedCP>(&src);
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
DgnDbStatus TextAnnotationSeed::_OnDelete() const
    {
    return DgnDbStatus::DeletionProhibited; // purge only
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TextAnnotationSeedId TextAnnotationSeed::QuerySeedId(Code const& code, DgnDbR db)
    {
    return TextAnnotationSeedId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void TextAnnotationSeed::Reset()
    {
    InvalidateElementId();
    InvalidateCode();
    m_descr.clear();
    ResetProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TextAnnotationSeed::ResetProperties()
    {
    m_data.ClearAllProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextAnnotationSeed::ExistsById(TextAnnotationSeedId id, DgnDbR db)
    {
    PRECONDITION(id.IsValid(), false);

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationSeed) " WHERE ECInstanceId=? LIMIT 1");
    if (stmt.IsValid())
        {
        stmt->BindId(1, id);
        if (BE_SQLITE_ROW == stmt->Step())
            return 0 < stmt->GetValueInt(0);
        }

    return false;
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
AnnotationFrameStyleId TextAnnotationSeed::GetFrameStyleId() const { return AnnotationFrameStyleId((uint64_t)getIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE)); }
void TextAnnotationSeed::SetFrameStyleId(AnnotationFrameStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_LEADERSTYLEID_VALUE = 0;
AnnotationLeaderStyleId TextAnnotationSeed::GetLeaderStyleId() const { return AnnotationLeaderStyleId((uint64_t)getIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE)); }
void TextAnnotationSeed::SetLeaderStyleId(AnnotationLeaderStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_TEXTSTYLEID_VALUE = 0;
AnnotationTextStyleId TextAnnotationSeed::GetTextStyleId() const { return AnnotationTextStyleId((uint64_t)getIntegerValue(m_data, TextAnnotationSeedProperty::TextStyleId, DEFAULT_TEXTSTYLEID_VALUE)); }
void TextAnnotationSeed::SetTextStyleId(AnnotationTextStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::TextStyleId, DEFAULT_TEXTSTYLEID_VALUE, value.GetValue()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr TextAnnotationSeed::CreateEffectiveStyle(TextAnnotationSeedPropertyBagCR overrides) const
    {
    TextAnnotationSeedPtr clone = Clone();
    clone->InvalidateCode();
    clone->m_descr.clear();
    clone->InvalidateElementId();

    clone->m_data.MergeWith(overrides);

    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TextAnnotationSeed::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationSeed));
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? static_cast<size_t>(stmt->GetValueInt(0)) : 0;
    }

#define SELECT_TextAnnotationSeed "SELECT ECInstanceId, Code, Descr FROM " DGN_SCHEMA(DGN_CLASSNAME_TextAnnotationSeed)
#define SELECT_ORDERED_TextAnnotationSeed SELECT_TextAnnotationSeed " ORDER BY Code"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TextAnnotationSeed::Iterator TextAnnotationSeed::MakeIterator(DgnDbR db, bool ordered)
    {
    Utf8CP ecSql = ordered? SELECT_ORDERED_TextAnnotationSeed : SELECT_TextAnnotationSeed;

    Iterator iter;
    iter.Prepare(db, ecSql, 0);

    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TextAnnotationSeed::_GetMemSize() const
    {
    return T_Super::_GetMemSize() + m_data.GetMemSize() + static_cast<uint32_t>(m_descr.length());
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
BentleyStatus TextAnnotationSeedPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, TextAnnotationSeedCR style, FlatBufEncodeOptions options)
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
    style.Reset();
    
    auto fbStyle = GetRoot<FB::TextAnnotationSeed>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }

