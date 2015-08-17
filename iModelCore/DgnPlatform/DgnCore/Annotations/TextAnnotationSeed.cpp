//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotationSeed.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationSeed::TextAnnotationSeed(DgnDbR project) :
    T_Super()
    {
    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
void TextAnnotationSeed::CopyFrom(TextAnnotationSeedCR rhs)
    {
    m_dgndb = rhs.m_dgndb;
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
DgnStyleId TextAnnotationSeed::GetFrameStyleId() const { return DgnStyleId(getIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE)); }
void TextAnnotationSeed::SetFrameStyleId(DgnStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::FrameStyleId, DEFAULT_FRAMESTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_LEADERSTYLEID_VALUE = 0;
DgnStyleId TextAnnotationSeed::GetLeaderStyleId() const { return DgnStyleId(getIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE)); }
void TextAnnotationSeed::SetLeaderStyleId(DgnStyleId value) { setIntegerValue(m_data, TextAnnotationSeedProperty::LeaderStyleId, DEFAULT_LEADERSTYLEID_VALUE, value.GetValue()); }
    
static const TextAnnotationSeedPropertyBag::T_Integer DEFAULT_TEXTSTYLEID_VALUE = 0;
DgnStyleId TextAnnotationSeed::GetTextStyleId() const { return DgnStyleId(getIntegerValue(m_data, TextAnnotationSeedProperty::TextStyleId, DEFAULT_TEXTSTYLEID_VALUE)); }
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

#define DGN_STYLE_TYPE_TextAnnotationSeed "5"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr DgnTextAnnotationSeeds::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Name,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    TextAnnotationSeedPtr style = TextAnnotationSeed::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == TextAnnotationSeedPersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);

    style->SetDescription(query.GetValueText(1));
    style->SetId(id);
    style->SetName(query.GetValueText(0));

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TextAnnotationSeedPtr DgnTextAnnotationSeeds::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Id,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    TextAnnotationSeedPtr style = TextAnnotationSeed::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == TextAnnotationSeedPersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);
    
    style->SetDescription(query.GetValueText(1));
    style->SetId(query.GetValueId<DgnStyleId>(0));
    style->SetName(name);

    return style;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
DgnTextAnnotationSeeds::Iterator::const_iterator DgnTextAnnotationSeeds::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,Name,Descr FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed, true);
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
size_t DgnTextAnnotationSeeds::Iterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString("SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed, true);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    m_params.Bind(query);

    POSTCONDITION(BE_SQLITE_ROW == query.Step(), 0);

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnTextAnnotationSeeds::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return false;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnTextAnnotationSeeds::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextAnnotationSeeds::Insert(TextAnnotationSeedR style)
    {
    // Don't assert to ensure an invalid ID.
    // Consider the case of cloning a style object, modifying, and then inserting it as a new style. The Clone keeps the ID, and I don't think it's worth having an overload of Clone to expose this detail.

    bvector<Byte> data;
    PRECONDITION(SUCCESS == TextAnnotationSeedPersistence::EncodeAsFlatBuf(data, style, TextAnnotationSeedPersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyleId nextId;
    PRECONDITION(BE_SQLITE_OK == m_dgndb.GetServerIssuedId(nextId, DGN_TABLE(DGN_CLASSNAME_Style), "Id"), ERROR);
    
    Statement insert;
    insert.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Style) " (Id,Type,Name,Descr,Data) VALUES (?," DGN_STYLE_TYPE_TextAnnotationSeed ",?,?,?)");
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
BentleyStatus DgnTextAnnotationSeeds::Update(TextAnnotationSeedCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    
    bvector<Byte> data;
    PRECONDITION(SUCCESS == TextAnnotationSeedPersistence::EncodeAsFlatBuf(data, style, TextAnnotationSeedPersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    Statement update;
    update.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Style) " SET Name=?,Descr=?,Data=? WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed " AND Id=?");
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
BentleyStatus DgnTextAnnotationSeeds::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);

    Statement del;
    del.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_TextAnnotationSeed " AND Id=?");
    del.BindId(1, id);

    POSTCONDITION(BE_SQLITE_DONE == del.Step(), ERROR);

    return SUCCESS;
    }
