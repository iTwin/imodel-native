//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotationSeed.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPropertyBagPtr TextAnnotationSeedPropertyBag::Create() { return new TextAnnotationSeedPropertyBag(); }
TextAnnotationSeedPropertyBag::TextAnnotationSeedPropertyBag() :
    T_Super()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPropertyBagPtr TextAnnotationSeedPropertyBag::Clone() const { return new TextAnnotationSeedPropertyBag(*this); }
TextAnnotationSeedPropertyBag::TextAnnotationSeedPropertyBag(TextAnnotationSeedPropertyBagCR rhs) : T_Super(rhs) { }
TextAnnotationSeedPropertyBagR TextAnnotationSeedPropertyBag::operator=(TextAnnotationSeedPropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
bool TextAnnotationSeedPropertyBag::HasProperty(TextAnnotationSeedProperty key) const { return T_Super::HasProperty((T_Key)key); }
void TextAnnotationSeedPropertyBag::ClearProperty(TextAnnotationSeedProperty key) { T_Super::ClearProperty((T_Key)key); }
TextAnnotationSeedPropertyBag::T_Integer TextAnnotationSeedPropertyBag::GetIntegerProperty(TextAnnotationSeedProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
void TextAnnotationSeedPropertyBag::SetIntegerProperty(TextAnnotationSeedProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
TextAnnotationSeedPropertyBag::T_Real TextAnnotationSeedPropertyBag::GetRealProperty(TextAnnotationSeedProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
void TextAnnotationSeedPropertyBag::SetRealProperty(TextAnnotationSeedProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr TextAnnotationSeed::Create(DgnProjectR project) { return new TextAnnotationSeed(project); }
TextAnnotationSeed::TextAnnotationSeed(DgnProjectR project) :
    T_Super()
    {
    m_project = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr TextAnnotationSeed::Clone() const { return new TextAnnotationSeed(*this); }
TextAnnotationSeed::TextAnnotationSeed(TextAnnotationSeedCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
TextAnnotationSeedR TextAnnotationSeed::operator=(TextAnnotationSeedCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void TextAnnotationSeed::CopyFrom(TextAnnotationSeedCR rhs)
    {
    m_project = rhs.m_project;
    m_id = rhs.m_id;
    m_name = rhs.m_name;
    m_description = rhs.m_description;
    m_data = rhs.m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
void TextAnnotationSeed::Reset()
    {
    m_id.Invalidate();
    m_name.clear();
    m_description.clear();
    m_data.ClearAllProperties();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
DgnProjectR TextAnnotationSeed::GetDgnProjectR() const { return *m_project; }
DgnStyleId TextAnnotationSeed::GetId() const { return m_id; }
void TextAnnotationSeed::SetId(DgnStyleId value) { m_id = value; }
Utf8StringCR TextAnnotationSeed::GetName() const { return m_name; }
void TextAnnotationSeed::SetName(Utf8CP value) { m_name = value; }
Utf8StringCR TextAnnotationSeed::GetDescription() const { return m_description; }
void TextAnnotationSeed::SetDescription(Utf8CP value) { m_description = value; }

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
DgnStyleId TextAnnotationSeed::GetFrameStyleId() const { return DgnStyleId((UInt32)getIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE)); }
void TextAnnotationSeed::SetFrameStyleId(DgnStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_LEADERSTYLEID_VALUE = 0;
DgnStyleId TextAnnotationSeed::GetLeaderStyleId() const { return DgnStyleId((UInt32)getIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE)); }
void TextAnnotationSeed::SetLeaderStyleId(DgnStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_TEXTSTYLEID_VALUE = 0;
DgnStyleId TextAnnotationSeed::GetTextStyleId() const { return DgnStyleId((UInt32)getIntegerValue(m_data, TextAnnotationSeedProperty::TextStyleId, DEFAULT_TEXTSTYLEID_VALUE)); }
void TextAnnotationSeed::SetTextStyleId(DgnStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::TextStyleId, DEFAULT_TEXTSTYLEID_VALUE, value.GetValue()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr TextAnnotationSeed::CreateEffectiveStyle(TextAnnotationSeedPropertyBagCR overrides) const
    {
    TextAnnotationSeedPtr clone = Clone();
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
    Offset<String> nameOffset;
    Offset<String> descriptionOffset;
    if (!isEnumFlagSet(FlatBufEncodeOptions::ExcludeNonPropertyData, options))
        {
        PRECONDITION(!style.m_name.empty(), ERROR);

        nameOffset = encoder.CreateString(style.m_name);
        
        if (!style.m_description.empty())
            descriptionOffset = encoder.CreateString(style.m_description);
        }

    FB::TextAnnotationSeedSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data), ERROR);

    FB::TextAnnotationSeedSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);

    //.............................................................................................
    FB::TextAnnotationSeedBuilder fbStyle(encoder);
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

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr DgnTextAnnotationSeeds::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), NULL);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::TextAnnotationSeed, id);
    if (!styleRow.GetId().IsValid())
        return NULL;

    TextAnnotationSeedPtr style = TextAnnotationSeed::Create(m_project);
    
    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Therefore, do this before setting the fields from the table.
    POSTCONDITION(SUCCESS == TextAnnotationSeedPersistence::DecodeFromFlatBuf(*style, (ByteCP)styleRow.GetData(), styleRow.GetDataSize()), NULL);
    
    style->SetDescription(styleRow.GetDescription());
    style->SetId(id);
    style->SetName(styleRow.GetName());

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr DgnTextAnnotationSeeds::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), NULL);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::TextAnnotationSeed, name);
    if (!id.IsValid())
        return NULL;

    return QueryById(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
bool DgnTextAnnotationSeeds::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::TextAnnotationSeed, id);
    
    return styleRow.GetId().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
bool DgnTextAnnotationSeeds::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::TextAnnotationSeed, name);
    
    return id.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr DgnTextAnnotationSeeds::Insert(TextAnnotationSeedCR style)
    {
    PRECONDITION(!style.GetName().empty(), NULL);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, NULL);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == TextAnnotationSeedPersistence::EncodeAsFlatBuf(styleData, style, TextAnnotationSeedPersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), NULL);

    DgnStyles::Style styleRow(DgnStyleId(), DgnStyleType::TextAnnotationSeed, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().InsertStyle(styleRow)))
        return NULL;

    return QueryById(styleRow.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextAnnotationSeeds::Update(TextAnnotationSeedCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    PRECONDITION(!style.GetName().empty(), ERROR);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, ERROR);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == TextAnnotationSeedPersistence::EncodeAsFlatBuf(styleData, style, TextAnnotationSeedPersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyles::Style styleRow(style.GetId(), DgnStyleType::TextAnnotationSeed, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().UpdateStyle(styleRow)))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextAnnotationSeeds::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);
    
    POSTCONDITION(BE_SQLITE_DONE == m_project.Styles().DeleteStyle(DgnStyleType::TextAnnotationSeed, id), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
DgnStyles::Iterator DgnTextAnnotationSeeds::MakeIterator(DgnStyleSort sortOrder) const
    {
    Utf8String queryModifierClauses;
    queryModifierClauses.Sprintf("WHERE Type=%d", DgnStyleType::TextAnnotationSeed);

    switch (sortOrder)
        {
        case DgnStyleSort::None:    break;
        case DgnStyleSort::NameAsc: queryModifierClauses += " ORDER BY Name ASC";   break;
        case DgnStyleSort::NameDsc: queryModifierClauses += " ORDER BY Name DESC";  break;

        default:
            BeAssert(false); // Unknown/unexpected DgnStyleSort
            break;
        }

    DgnStyles::Iterator it(m_project);
    it.Params().SetWhere(queryModifierClauses.c_str());
    return it;
    }
