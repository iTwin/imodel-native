//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationLeaderStyle.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderStylePersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationLeaderStylePropertyBag::_IsIntegerProperty(T_Key key) const
    {
    switch ((AnnotationLeaderStyleProperty)key)
        {
        case AnnotationLeaderStyleProperty::LineColorId:
        case AnnotationLeaderStyleProperty::LineStyle:
        case AnnotationLeaderStyleProperty::LineType:
        case AnnotationLeaderStyleProperty::LineWeight:
        case AnnotationLeaderStyleProperty::TerminatorColorId:
        case AnnotationLeaderStyleProperty::TerminatorStyle:
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePropertyBagPtr AnnotationLeaderStylePropertyBag::Create() { return new AnnotationLeaderStylePropertyBag(); }
AnnotationLeaderStylePropertyBag::AnnotationLeaderStylePropertyBag() :
    T_Super()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePropertyBagPtr AnnotationLeaderStylePropertyBag::Clone() const { return new AnnotationLeaderStylePropertyBag(*this); }
AnnotationLeaderStylePropertyBag::AnnotationLeaderStylePropertyBag(AnnotationLeaderStylePropertyBagCR rhs) : T_Super(rhs) { }
AnnotationLeaderStylePropertyBagR AnnotationLeaderStylePropertyBag::operator=(AnnotationLeaderStylePropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool AnnotationLeaderStylePropertyBag::HasProperty(AnnotationLeaderStyleProperty key) const { return T_Super::HasProperty((T_Key)key); }
void AnnotationLeaderStylePropertyBag::ClearProperty(AnnotationLeaderStyleProperty key) { T_Super::ClearProperty((T_Key)key); }
AnnotationLeaderStylePropertyBag::T_Integer AnnotationLeaderStylePropertyBag::GetIntegerProperty(AnnotationLeaderStyleProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
void AnnotationLeaderStylePropertyBag::SetIntegerProperty(AnnotationLeaderStyleProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
AnnotationLeaderStylePropertyBag::T_Real AnnotationLeaderStylePropertyBag::GetRealProperty(AnnotationLeaderStyleProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
void AnnotationLeaderStylePropertyBag::SetRealProperty(AnnotationLeaderStyleProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr AnnotationLeaderStyle::Create(DgnProjectR project) { return new AnnotationLeaderStyle(project); }
AnnotationLeaderStyle::AnnotationLeaderStyle(DgnProjectR project) :
    T_Super()
    {
    m_project = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr AnnotationLeaderStyle::Clone() const { return new AnnotationLeaderStyle(*this); }
AnnotationLeaderStyle::AnnotationLeaderStyle(AnnotationLeaderStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationLeaderStyleR AnnotationLeaderStyle::operator=(AnnotationLeaderStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationLeaderStyle::CopyFrom(AnnotationLeaderStyleCR rhs)
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
DgnProjectR AnnotationLeaderStyle::GetDgnProjectR() const { return *m_project; }
DgnStyleId AnnotationLeaderStyle::GetId() const { return m_id; }
void AnnotationLeaderStyle::SetId(DgnStyleId value) { m_id = value; }
Utf8StringCR AnnotationLeaderStyle::GetName() const { return m_name; }
void AnnotationLeaderStyle::SetName(Utf8CP value) { m_name = value; }
Utf8StringCR AnnotationLeaderStyle::GetDescription() const { return m_description; }
void AnnotationLeaderStyle::SetDescription(Utf8CP value) { m_description = value; }

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
static const UInt32 DEFAULT_LINECOLORID_VALUE = 0;
UInt32 AnnotationLeaderStyle::GetLineColorId() const { return (UInt32)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColorId, DEFAULT_LINECOLORID_VALUE); }
void AnnotationLeaderStyle::SetLineColorId(UInt32 value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineColorId, DEFAULT_LINECOLORID_VALUE, value); }

static const UInt32 DEFAULT_LINESTYLE_VALUE = 0;
Int32 AnnotationLeaderStyle::GetLineStyle() const { return (Int32)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineStyle, DEFAULT_LINESTYLE_VALUE); }
void AnnotationLeaderStyle::SetLineStyle(Int32 value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineStyle, DEFAULT_LINESTYLE_VALUE, (UInt32)value); }

static const UInt32 DEFAULT_LINETYPE_VALUE = (UInt32)AnnotationLeaderLineType::None;
AnnotationLeaderLineType AnnotationLeaderStyle::GetLineType() const { return (AnnotationLeaderLineType)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineType, DEFAULT_LINETYPE_VALUE); }
void AnnotationLeaderStyle::SetLineType(AnnotationLeaderLineType value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineType, DEFAULT_LINETYPE_VALUE, (UInt32)value); }

static const UInt32 DEFAULT_LINEWEIGHT_VALUE = 0;
UInt32 AnnotationLeaderStyle::GetLineWeight() const { return (UInt32)getIntegerValue(m_data, AnnotationLeaderStyleProperty::LineWeight, DEFAULT_LINEWEIGHT_VALUE); }
void AnnotationLeaderStyle::SetLineWeight(UInt32 value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::LineWeight, DEFAULT_LINEWEIGHT_VALUE, value); }

static const UInt32 DEFAULT_TERMINATORCOLORID_VALUE = 0;
UInt32 AnnotationLeaderStyle::GetTerminatorColorId() const { return (UInt32)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColorId, DEFAULT_TERMINATORCOLORID_VALUE); }
void AnnotationLeaderStyle::SetTerminatorColorId(UInt32 value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorColorId, DEFAULT_TERMINATORCOLORID_VALUE, value); }

static const double DEFAULT_TERMINATORSCALEFACTOR_VALUE = 1.0;
double AnnotationLeaderStyle::GetTerminatorScaleFactor() const { return getRealValue(m_data, AnnotationLeaderStyleProperty::TerminatorScaleFactor, DEFAULT_TERMINATORSCALEFACTOR_VALUE); }
void AnnotationLeaderStyle::SetTerminatorScaleFactor(double value) { setRealValue(m_data, AnnotationLeaderStyleProperty::TerminatorScaleFactor, DEFAULT_TERMINATORSCALEFACTOR_VALUE, value); }

static const UInt32 DEFAULT_TERMINATORSTYLE_VALUE = 0;
Int32 AnnotationLeaderStyle::GetTerminatorStyle() const { return (Int32)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorStyle, DEFAULT_TERMINATORSTYLE_VALUE); }
void AnnotationLeaderStyle::SetTerminatorStyle(Int32 value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorStyle, DEFAULT_TERMINATORSTYLE_VALUE, (UInt32)value); }

static const UInt32 DEFAULT_TERMINATORTYPE_VALUE = (UInt32)AnnotationLeaderTerminatorType::None;
AnnotationLeaderTerminatorType AnnotationLeaderStyle::GetTerminatorType() const { return (AnnotationLeaderTerminatorType)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorType, DEFAULT_TERMINATORTYPE_VALUE); }
void AnnotationLeaderStyle::SetTerminatorType(AnnotationLeaderTerminatorType value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorType, DEFAULT_TERMINATORTYPE_VALUE, (UInt32)value); }

static const UInt32 DEFAULT_TERMINATORWEIGHT_VALUE = 0;
UInt32 AnnotationLeaderStyle::GetTerminatorWeight() const { return (UInt32)getIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorWeight, DEFAULT_TERMINATORWEIGHT_VALUE); }
void AnnotationLeaderStyle::SetTerminatorWeight(UInt32 value) { setIntegerValue(m_data, AnnotationLeaderStyleProperty::TerminatorWeight, DEFAULT_TERMINATORWEIGHT_VALUE, value); }

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

static const UInt32 CURRENT_MAJOR_VERSION = 1;
static const UInt32 CURRENT_MINOR_VERSION = 0;

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
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineColorId, FB::AnnotationLeaderStyleProperty_LineColorId, DEFAULT_LINECOLORID_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineStyle, FB::AnnotationLeaderStyleProperty_LineStyle, DEFAULT_LINESTYLE_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineType, FB::AnnotationLeaderStyleProperty_LineType, DEFAULT_LINETYPE_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::LineWeight, FB::AnnotationLeaderStyleProperty_LineWeight, DEFAULT_LINEWEIGHT_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorColorId, FB::AnnotationLeaderStyleProperty_TerminatorColorId, DEFAULT_TERMINATORCOLORID_VALUE);
    appendRealSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorScaleFactor, FB::AnnotationLeaderStyleProperty_TerminatorScaleFactor, DEFAULT_TERMINATORSCALEFACTOR_VALUE);
    appendIntegerSetter(setters, data, AnnotationLeaderStyleProperty::TerminatorStyle, FB::AnnotationLeaderStyleProperty_TerminatorStyle, DEFAULT_TERMINATORSTYLE_VALUE);
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
            case FB::AnnotationLeaderStyleProperty_LineColorId: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineColorId, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineStyle: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineStyle, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineType: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineType, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_LineWeight: data.SetIntegerProperty(AnnotationLeaderStyleProperty::LineWeight, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorColorId: data.SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorColorId, setter.integerValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorScaleFactor: data.SetRealProperty(AnnotationLeaderStyleProperty::TerminatorScaleFactor, setter.realValue()); break;
            case FB::AnnotationLeaderStyleProperty_TerminatorStyle: data.SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorStyle, setter.integerValue()); break;
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
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr DgnAnnotationLeaderStyles::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), NULL);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::AnnotationLeader, id);
    if (!styleRow.GetId().IsValid())
        return NULL;

    AnnotationLeaderStylePtr style = AnnotationLeaderStyle::Create(m_project);
    
    // IMPORTANT: Decoding "resets" the style object to ensure defaults for non-persisted values.
    // Therefore, do this before setting the fields from the table.
    POSTCONDITION(SUCCESS == AnnotationLeaderStylePersistence::DecodeFromFlatBuf(*style, (ByteCP)styleRow.GetData(), styleRow.GetDataSize()), NULL);
    
    style->SetDescription(styleRow.GetDescription());
    style->SetId(id);
    style->SetName(styleRow.GetName());

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr DgnAnnotationLeaderStyles::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), NULL);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::AnnotationLeader, name);
    if (!id.IsValid())
        return NULL;

    return QueryById(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool DgnAnnotationLeaderStyles::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    DgnStyles::Style styleRow = m_project.Styles().QueryStyleById(DgnStyleType::AnnotationLeader, id);
    
    return styleRow.GetId().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
bool DgnAnnotationLeaderStyles::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    DgnStyleId id = m_project.Styles().QueryStyleId(DgnStyleType::AnnotationLeader, name);
    
    return id.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr DgnAnnotationLeaderStyles::Insert(AnnotationLeaderStyleCR style)
    {
    PRECONDITION(!style.GetName().empty(), NULL);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, NULL);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == AnnotationLeaderStylePersistence::EncodeAsFlatBuf(styleData, style, AnnotationLeaderStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), NULL);

    DgnStyles::Style styleRow(DgnStyleId(), DgnStyleType::AnnotationLeader, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().InsertStyle(styleRow)))
        return NULL;

    return QueryById(styleRow.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationLeaderStyles::Update(AnnotationLeaderStyleCR style)
    {
    PRECONDITION(style.GetId().IsValid(), ERROR);
    PRECONDITION(!style.GetName().empty(), ERROR);
    PRECONDITION(&style.GetDgnProjectR() == &m_project, ERROR);
    
    bvector<Byte> styleData;
    POSTCONDITION(SUCCESS == AnnotationLeaderStylePersistence::EncodeAsFlatBuf(styleData, style, AnnotationLeaderStylePersistence::FlatBufEncodeOptions::ExcludeNonPropertyData), ERROR);

    DgnStyles::Style styleRow(style.GetId(), DgnStyleType::AnnotationLeader, style.GetName().c_str(), style.GetDescription().c_str(), &styleData[0], styleData.size());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().UpdateStyle(styleRow)))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnAnnotationLeaderStyles::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);
    
    POSTCONDITION(BE_SQLITE_DONE == m_project.Styles().DeleteStyle(DgnStyleType::AnnotationLeader, id), ERROR);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
DgnStyles::Iterator DgnAnnotationLeaderStyles::MakeIterator(DgnStyleSort sortOrder) const
    {
    Utf8String queryModifierClauses;
    queryModifierClauses.Sprintf("WHERE Type=%d", DgnStyleType::AnnotationLeader);

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
