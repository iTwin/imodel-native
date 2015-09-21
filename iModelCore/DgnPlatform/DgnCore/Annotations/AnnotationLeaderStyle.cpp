//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationLeaderStyle.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationLeaderStylePropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((AnnotationLeaderStyleProperty)key)
        {
        case AnnotationLeaderStyleProperty::LineColor:
        case AnnotationLeaderStyleProperty::LineType:
        case AnnotationLeaderStyleProperty::LineWeight:
        case AnnotationLeaderStyleProperty::TerminatorColor:
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStyle::AnnotationLeaderStyle(DgnDbR project) :
    T_Super()
    {
    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationLeaderStyle::CopyFrom(AnnotationLeaderStyleCR rhs)
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
void AnnotationLeaderStyle::Reset()
    {
    m_id.Invalidate();
    m_name.clear();
    m_description.clear();
    m_data.ClearAllProperties();
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
static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_LINECOLOR_VALUE = ElementColor().ToInt64();
ElementColor AnnotationLeaderStyle::GetLineColor() const { return ElementColor(getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColor, DEFAULT_LINECOLOR_VALUE)); }
void AnnotationLeaderStyle::SetLineColor(ElementColor value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColor, DEFAULT_LINECOLOR_VALUE, value.ToInt64()); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_LINETYPE_VALUE = (AnnotationLeaderStylePropertyBag::T_Integer)AnnotationLeaderLineType::None;
AnnotationLeaderLineType AnnotationLeaderStyle::GetLineType() const { return (AnnotationLeaderLineType)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineType, DEFAULT_LINETYPE_VALUE); }
void AnnotationLeaderStyle::SetLineType(AnnotationLeaderLineType value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineType, DEFAULT_LINETYPE_VALUE, (uint32_t)value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_LINEWEIGHT_VALUE = 0;
uint32_t AnnotationLeaderStyle::GetLineWeight() const { return (uint32_t)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineWeight, DEFAULT_LINEWEIGHT_VALUE); }
void AnnotationLeaderStyle::SetLineWeight(uint32_t value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineWeight, DEFAULT_LINEWEIGHT_VALUE, value); }

static const AnnotationLeaderStylePropertyBag::T_Integer DEFAULT_TERMINATORCOLOR_VALUE = ElementColor().ToInt64();
ElementColor AnnotationLeaderStyle::GetTerminatorColor() const { return ElementColor(getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColor, DEFAULT_TERMINATORCOLOR_VALUE)); }
void AnnotationLeaderStyle::SetTerminatorColor(ElementColor value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColor, DEFAULT_TERMINATORCOLOR_VALUE, value.ToInt64()); }

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
    AnnotationLeaderStylePtr clone = Clone();
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
FB::AnnotationLeaderStyleSetters& setters,
AnnotationLeaderStylePropertyBagCR data,
AnnotationLeaderStyleProperty tsProp,
decltype(declval<FB::AnnotationLeaderStyleSetter>().key()) fbProp,
decltype(declval<FB::AnnotationLeaderStyleSetter>().integerValue()) defaultValue
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetIntegerProperty(tsProp);
    if (value == defaultValue)
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
decltype(declval<FB::AnnotationLeaderStyleSetter>().realValue()) defaultValue
)
    {
    if (!data.HasProperty(tsProp))
        return;

    auto value = data.GetRealProperty(tsProp);
    if (value == defaultValue)
        return;

    setters.push_back(FB::AnnotationLeaderStyleSetter(fbProp, 0, value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderStylePersistence::EncodeAsFlatBuf(FB::AnnotationLeaderStyleSetters& setters, AnnotationLeaderStylePropertyBagCR data)
    {
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineColor, FB::AnnotationLeaderStyleProperty_LineColor, DEFAULT_LINECOLOR_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineType, FB::AnnotationLeaderStyleProperty_LineType, DEFAULT_LINETYPE_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineWeight, FB::AnnotationLeaderStyleProperty_LineWeight, DEFAULT_LINEWEIGHT_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorColor, FB::AnnotationLeaderStyleProperty_TerminatorColor, DEFAULT_TERMINATORCOLOR_VALUE);
    appendRealSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorScaleFactor, FB::AnnotationLeaderStyleProperty_TerminatorScaleFactor, DEFAULT_TERMINATORSCALEFACTOR_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorType, FB::AnnotationLeaderStyleProperty_TerminatorType, DEFAULT_TERMINATORTYPE_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorWeight, FB::AnnotationLeaderStyleProperty_TerminatorWeight, DEFAULT_TERMINATORWEIGHT_VALUE);

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
    Offset<String> nameOffset;
    Offset<String> descriptionOffset;
    if (!isEnumFlagSet(FlatBufEncodeOptions::ExcludeNonPropertyData, options))
        {
        PRECONDITION(!style.m_name.empty(), ERROR);

        nameOffset = encoder.CreateString(style.m_name);
        
        if (!style.m_description.empty())
            descriptionOffset = encoder.CreateString(style.m_description);
        }

    FB::AnnotationLeaderStyleSetters setters;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(setters, style.m_data), ERROR);

    FB::AnnotationLeaderStyleSetterVectorOffset settersOffset;
    if (!setters.empty())
        settersOffset = encoder.CreateVectorOfStructs(setters);
    
    //.............................................................................................
    FB::AnnotationLeaderStyleBuilder fbStyle(encoder);
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
BentleyStatus AnnotationLeaderStylePersistence::DecodeFromFlatBuf(AnnotationLeaderStylePropertyBagR data, FB::AnnotationLeaderStyleSetterVector const& setters)
    {
    for (auto const& setter : setters)
        {
        switch (setter.key())
            {
            case FB::AnnotationLeaderStyleProperty_LineColor: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineColor, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineType: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineType, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineWeight: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineWeight, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorColor: data.SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorColor, setter.integerValue()); break;
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
    style.Reset();
    
    auto fbStyle = GetRoot<FB::AnnotationLeaderStyle>(buffer);

    PRECONDITION(fbStyle->has_majorVersion(), ERROR);
    if (fbStyle->majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbStyle->has_id())
        style.m_id = DgnStyleId((uint64_t)fbStyle->id());

    if (fbStyle->has_name())
        style.m_name = fbStyle->name()->c_str();

    if (fbStyle->has_description())
        style.m_description = fbStyle->description()->c_str();

    if (fbStyle->has_setters())
        POSTCONDITION(SUCCESS == DecodeFromFlatBuf(style.m_data, *fbStyle->setters()), ERROR);

    return SUCCESS;
    }

#define DGN_STYLE_TYPE_AnnotationLeader "4"
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr DgnAnnotationLeaderStyles::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Name,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    AnnotationLeaderStylePtr style = AnnotationLeaderStyle::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == AnnotationLeaderStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);

    style->SetDescription(query.GetValueText(1));
    style->SetId(id);
    style->SetName(query.GetValueText(0));

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr DgnAnnotationLeaderStyles::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), nullptr);

    Statement query;
    query.Prepare(m_dgndb, "SELECT Id,Descr,Data FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return nullptr;

    AnnotationLeaderStylePtr style = AnnotationLeaderStyle::Create(m_dgndb);

    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Since we store name and description independently, we must decode first, otherwise they'll be reset.
    POSTCONDITION(SUCCESS == AnnotationLeaderStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)query.GetValueBlob(2), (size_t)query.GetColumnBytes(2)), nullptr);
    
    style->SetDescription(query.GetValueText(1));
    style->SetId(query.GetValueId<DgnStyleId>(0));
    style->SetName(name);

    return style;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2014
//---------------------------------------------------------------------------------------
DgnAnnotationLeaderStyles::Iterator::const_iterator DgnAnnotationLeaderStyles::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sql = MakeSqlString("SELECT Id,Name,Descr FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader, true);
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
size_t DgnAnnotationLeaderStyles::Iterator::QueryCount() const
    {
    Utf8String sql = MakeSqlString("SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader, true);

    Statement query;
    query.Prepare(*m_db, sql.c_str());
    m_params.Bind(query);

    POSTCONDITION(BE_SQLITE_ROW == query.Step(), 0);

    return (size_t)query.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnAnnotationLeaderStyles::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader " AND Id=?");
    query.BindId(1, id);

    if (BE_SQLITE_ROW != query.Step())
        return false;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
bool DgnAnnotationLeaderStyles::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    Statement query;
    query.Prepare(m_dgndb, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader " AND Name=?");
    query.BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != query.Step())
        return false;

    return (query.GetValueInt(0) > 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationLeaderStyles::Insert(AnnotationLeaderStyleR style)
    {
    // Don't assert to ensure an invalid ID.
    // Consider the case of cloning a style object, modifying, and then inserting it as a new style. The Clone keeps the ID, and I don't think it's worth having an overload of Clone to expose this detail.

    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationLeaderStylePersistence::EncodeAsFlatBuf(data, style, AnnotationLeaderStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyleId nextId;
    PRECONDITION(BE_SQLITE_OK == m_dgndb.GetServerIssuedId(nextId, DGN_TABLE(DGN_CLASSNAME_Style), "Id"), ERROR);
    
    Statement insert;
    insert.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Style) " (Id,Type,Name,Descr,Data) VALUES (?," DGN_STYLE_TYPE_AnnotationLeader ",?,?,?)");
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
BentleyStatus DgnAnnotationLeaderStyles::Update(AnnotationLeaderStyleCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    
    bvector<Byte> data;
    PRECONDITION(SUCCESS == AnnotationLeaderStylePersistence::EncodeAsFlatBuf(data, style, AnnotationLeaderStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    Statement update;
    update.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Style) " SET Name=?,Descr=?,Data=? WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader " AND Id=?");
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
BentleyStatus DgnAnnotationLeaderStyles::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);

    Statement del;
    del.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Style) " WHERE Type=" DGN_STYLE_TYPE_AnnotationLeader " AND Id=?");
    del.BindId(1, id);

    POSTCONDITION(BE_SQLITE_DONE == del.Step(), ERROR);

    return SUCCESS;
    }
