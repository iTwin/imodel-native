/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsDb.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

Utf8CP LsJsonHelpers::CompId                = "compId";

#define PROPNAME_Descr "Descr"
#define PROPNAME_Data "Data"

static Utf8CP DGNPROPERTYBLOB_CompId                = "compId";
static Utf8CP DGNPROPERTYBLOB_CompType              = "compType";
static Utf8CP DGNPROPERTYBLOB_Flags                 = "flags";
static Utf8CP DGNPROPERTYBLOB_UnitDef               = "unitDef";

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
double LsJsonHelpers::GetDouble(JsonValueCR json, CharCP fieldName, double defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
uint32_t LsJsonHelpers::GetUInt32(JsonValueCR json, CharCP fieldName, uint32_t defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
int32_t LsJsonHelpers::GetInt32(JsonValueCR json, CharCP fieldName, int32_t defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
uint64_t LsJsonHelpers::GetUInt64(JsonValueCR json, CharCP fieldName, uint64_t defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asUInt64();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
Utf8String LsJsonHelpers::GetString(JsonValueCR json, CharCP fieldName, CharCP defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentId LsJsonHelpers::GetComponentId(JsonValueCR json, CharCP typeName, CharCP idName, LsComponentType defaultType)
    {
    uint32_t idValue = GetUInt32(json, idName, 0);
    int32_t typeValue = GetInt32(json, typeName, (int32_t)defaultType);
    return LsComponentId(LsComponentType(typeValue), idValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void V10ComponentBase::GetDescription (Utf8P target) const
    {
    BeStringUtilities::Strncpy(target, LS_MAX_DESCR, m_descr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2015
//---------------------------------------------------------------------------------------
void V10ComponentBase::SetVersion()
    {
    m_version = InitialDgnDb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void V10ComponentBase::SetDescription (Utf8CP source)
    {
    memset (m_descr, 0, sizeof (m_descr));
    BeStringUtilities::Strncpy(m_descr, LS_MAX_DESCR, source);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsStrokePatternComponent::_Import(DgnImportContext& importer) const
    {
    LsStrokePatternComponentP result = new LsStrokePatternComponent(this);

    Json::Value     jsonValue;
    result->SaveToJson(jsonValue);
    LsComponentId componentId;
    LsComponent::AddComponentAsJsonProperty(componentId, importer.GetDestinationDb(), LsComponentType::LineCode, jsonValue);

    //  Rely on LsComponent::Import to record the component ID mapping.
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsPointComponent::_Import(DgnImportContext& importer) const
    {
    LsPointComponentP cloned = new LsPointComponent(*this, false);
    if (cloned->m_strokeComponent.IsValid())
        {
        LsComponentPtr ptr = LsComponent::GetImportedComponent(cloned->m_strokeComponent->GetId(), importer);
        cloned->m_strokeComponent = dynamic_cast<LsStrokePatternComponent*>(ptr.get());
        }

    for (LsSymbolReference& symbref : cloned->m_symbols)
        {
        if (symbref.m_symbol.IsValid())
            {
            LsComponentPtr ptr = LsComponent::GetImportedComponent(symbref.m_symbol->GetId(), importer);
            symbref.m_symbol  = dynamic_cast<LsSymbolComponentP>(ptr.get());
            }
        }

    Json::Value     jsonValue;
    cloned->SaveToJson(jsonValue);
    LsComponentId componentId;
    LsComponent::AddComponentAsJsonProperty(componentId, importer.GetDestinationDb(), LsComponentType::LinePoint, jsonValue);

    return cloned;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsCompoundComponent::SaveToJson(Json::Value& result) const
    {
    LsComponent::SaveToJson(result);

    Json::Value components(Json::arrayValue);
    uint32_t index = 0;
    for (LsOffsetComponent const& offset: m_components)
        {
        if (!offset.m_subComponent.IsValid())
            continue;
        Json::Value  entry(Json::objectValue);
        if (offset.m_offset != 0)
            entry["offset"] = offset.m_offset;
        LsComponentId id = offset.m_subComponent->GetId();
        entry["id"] = id.GetValue();
        entry["type"] = (int)id.GetType();
        components[index++] = entry;
        }

    result["comps"]=components;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LsCompoundComponent::CreateFromJson(LsCompoundComponentP* newCompound, Json::Value const & jsonDef, LsLocationCP thisLocation)
    {
    LsCompoundComponentP comp = new LsCompoundComponent(thisLocation);
    comp->ExtractDescription(jsonDef);

    JsonValueCR components = jsonDef["comps"];
    uint32_t nComponents = components.size();

    for (uint32_t i = 0; i < nComponents; i++)
        {
        JsonValueCR curr = components[i];
        double offset = LsJsonHelpers::GetDouble(curr, "offset", 0.0);
        LsComponentId id = LsJsonHelpers::GetComponentId(curr, "type", "id");
        LsLocation childLocation;
        childLocation.SetLocation(*thisLocation->GetDgnDb(), id);
        LsComponentPtr child = DgnLineStyles::GetLsComponent(childLocation);
        if (child.IsValid())
            comp->AppendComponent(*child, offset);
        }

    *newCompound = comp;
    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsPointComponent::SaveLineCodeIdToJson(JsonValueR json, LsComponentId patternId)
    {
    if (patternId.GetType() != LsComponentType::LineCode)
        json["lcType"] = (int)patternId.GetType();
    json["lcId"] = patternId.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsPointComponent::SaveSymbolIdToJson(JsonValueR json, LsComponentId symbolId)
    {
    if (symbolId.GetType() != LsComponentType::PointSymbol)
        json["symType"] = (int)symbolId.GetType();

    json["symId"] = symbolId.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsPointComponent::SaveToJson(Json::Value& result) const
    {
    LsComponent::SaveToJson(result);

    LsStrokePatternComponent const* strokePattern = GetStrokeComponentCP();
    BeAssert(nullptr != strokePattern);
    if (nullptr == strokePattern)
        return;

    LsComponentId  pattern = strokePattern->GetId();
    SaveLineCodeIdToJson(result, pattern);

    Json::Value symbols(Json::arrayValue);
    uint32_t index = 0;
    for (LsSymbolReference const& symRef: m_symbols)
        {
        Json::Value  entry(Json::objectValue);

        LsSymbolComponentCP symbolComponent = symRef.GetSymbolComponentCP();
        BeAssert(symbolComponent != nullptr);
        if (nullptr == symbolComponent)
            continue;       //  NEEDSWORK_LINESTYLE report error

        LsComponentId symbolId = symbolComponent->GetId();
        SaveSymbolIdToJson(entry, symbolId);

        entry["strokeNum"] = symRef.GetStrokeNumber();
        if (symRef.GetXOffset() != 0.0)
            entry["xOffset"] = symRef.GetXOffset();
        if (symRef.GetYOffset() != 0.0)
            entry["yOffset"] = symRef.GetYOffset();
        if (symRef.GetAngle() != 0.0)
            entry["angle"] = symRef.GetAngle();

        if (symRef.GetMod1() != 0)
            entry["mod1"] = symRef.GetMod1();

        symbols[index++] = entry;
        }

    result["symbols"]=symbols;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LsPointComponent::CreateFromJson(LsPointComponentP*newPoint, Json::Value const & jsonDef, LsLocationCP thisLocation)
    {
    LsPointComponentP pPoint = new LsPointComponent(thisLocation);
    pPoint->ExtractDescription(jsonDef);

    //  Get the stroke pattern
    LsComponentId id = LsJsonHelpers::GetComponentId(jsonDef, "lcType", "lcId", LsComponentType::LineCode);
    LsLocation childLocation;
    childLocation.SetLocation(*thisLocation->GetDgnDb(), id);
    LsComponentPtr child = DgnLineStyles::GetLsComponent(childLocation);
    pPoint->m_strokeComponent = dynamic_cast<LsStrokePatternComponentP>(child.get());

    JsonValueCR symbols = jsonDef["symbols"];
    uint32_t limit = symbols.size();
    if (limit > 32)
        limit = 32;
    for (unsigned index = 0; index < limit; ++index)
        {
        JsonValueCR  entry = symbols[index];
        LsSymbolReference symbolRef;

        LsComponentId symbolId = LsJsonHelpers::GetComponentId(entry, "symType", "symId", LsComponentType::PointSymbol);
        LsLocation symbolLocation;
        symbolLocation.SetLocation(*thisLocation->GetDgnDb(), symbolId);
        LsSymbolComponentP symbolComponent = dynamic_cast<LsSymbolComponentP>(DgnLineStyles::GetLsComponent(symbolLocation));

        BeAssert(symbolComponent != nullptr);
        if (nullptr == symbolComponent)
            continue;   //  NEEDSWORK_LINESTYLES report error

        symbolRef.SetSymbolComponent(*symbolComponent);
        symbolRef.SetStrokeNumber(LsJsonHelpers::GetInt32(entry, "strokeNum", 0));
        symbolRef.SetXOffset(LsJsonHelpers::GetDouble(entry, "xOffset", 0.0));
        symbolRef.SetYOffset(LsJsonHelpers::GetDouble(entry, "yOffset", 0.0));
        symbolRef.SetAngle(LsJsonHelpers::GetDouble(entry, "angle", 0.0));
        symbolRef.m_mod1 = LsJsonHelpers::GetUInt32(entry, "mod1", 0);

        pPoint->m_symbols.push_back(symbolRef);
        }

    *newPoint = pPoint;
    return LINESTYLE_STATUS_Success; 
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsCompoundComponent::_Import(DgnImportContext& importer) const
    {
    LsCompoundComponentP    result = new LsCompoundComponent(*this);

    for (auto& compOffset: result->m_components)
        compOffset.m_subComponent = LsComponent::GetImportedComponent(compOffset.m_subComponent->GetId(), importer);


    Json::Value     jsonValue;
    result->SaveToJson(jsonValue);
    LsComponentId componentId;
    LsComponent::AddComponentAsJsonProperty(componentId, importer.GetDestinationDb(), LsComponentType::Compound, jsonValue);

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsSymbolComponent::_Import(DgnImportContext& importer) const
    {
    LsSymbolComponentP result = new LsSymbolComponent(*this);

    importer.RemapGeometryPartId(result->m_geomPartId);

    Json::Value     jsonValue;
    result->SaveToJson(jsonValue);
    LsComponentId componentId;
    LsComponent::AddComponentAsJsonProperty(componentId, importer.GetDestinationDb(), LsComponentType::PointSymbol, jsonValue);

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LsSymbolComponent::CreateFromJson(LsSymbolComponentP*newComp, Json::Value const & jsonDef, LsLocationCP location)
    {
    LsSymbolComponentP pSym = new LsSymbolComponent(location);
    pSym->ExtractDescription(jsonDef);

    pSym->m_symBase.x = LsJsonHelpers::GetDouble(jsonDef, "baseX", 0.0);
    pSym->m_symBase.y = LsJsonHelpers::GetDouble(jsonDef, "baseY", 0.0);
    pSym->m_symBase.z = LsJsonHelpers::GetDouble(jsonDef, "baseZ",  0.0);

    pSym->m_symSize.x = LsJsonHelpers::GetDouble(jsonDef, "sizeX", 0.0);
    pSym->m_symSize.y = LsJsonHelpers::GetDouble(jsonDef, "sizeY", 0.0);
    pSym->m_symSize.z = LsJsonHelpers::GetDouble(jsonDef, "sizeZ",  0.0);

    pSym->m_geomPartId = DgnGeometryPartId(LsJsonHelpers::GetUInt64(jsonDef, "geomPartId", 0));
    pSym->m_symFlags = LsJsonHelpers::GetUInt32(jsonDef, "symFlags", 0);
    pSym->m_storedScale = LsJsonHelpers::GetDouble(jsonDef, "scale", 0.0);
    pSym->m_lineColorByLevel = LsJsonHelpers::GetInt32(jsonDef, "colorBySubCat", 0) != 0;
    pSym->m_lineColor = ColorDef(LsJsonHelpers::GetUInt32(jsonDef, "color", 0));
    pSym->m_fillColor = ColorDef(LsJsonHelpers::GetUInt32(jsonDef, "fillColor", 0));
    pSym->m_weight = LsJsonHelpers::GetUInt32(jsonDef, "weight", 0);

#if defined(NEEDSWORKLINESTYLE_FILLCOLOR)
    // If we don't have a solid color fill, set fill to match line color...
    if (FillDisplay::Never == params.GetFillDisplay() || nullptr != params.GetGradient())
        {
        if (params.IsLineColorFromSubCategoryAppearance())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: v10Symbol->m_fillBySubCategory = true;
        else
            v10Symbol->m_fillColor = params.GetLineColor().GetValue();
        }
    else
        {
        if (params.IsFillColorFromSubCategoryAppearance())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: v10Symbol->m_fillBySubCategory = true;
        else if (params.IsFillColorFromViewBackground())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: Do we need to support bg color fill for symbols?
        else
            v10Symbol->m_fillColor = params.GetFillColor().GetValue();
        }
#endif


#if defined(NEEDSWORKLINESTYLE_FILLCOLOR)
    if (params.IsWeightFromSubCategoryAppearance())
        v10Symbol->m_weight = 0; // NEEDSWORK: v10Symbol->m_weightBySubCategory = true;
    else
        v10Symbol->m_weight = params.GetWeight();
#endif
    *newComp = pSym;
    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsSymbolComponent::SaveSymbolDataToJson(Json::Value& result, DPoint3dCR base, DPoint3dCR size, DgnGeometryPartId const& geomPartId, int32_t flags, double storedScale, 
                                             bool colorBySubcategory, ColorDefCR lineColor, ColorDefCR fillColor, bool weightBySubcategory, int weight)
    {
    if (base.x != 0)
        result["baseX"] = base.x;
    if (base.y != 0)
        result["baseY"] = base.y;
    if (base.z != 0)
        result["baseZ"] = base.z;
    if (size.x != 0)
        result["sizeX"] = size.x;
    if (size.y != 0)
        result["sizeY"] = size.y;
    if (size.z != 0)
        result["sizeZ"] = size.z;

    result["geomPartId"] = geomPartId.GetValue();
    result["symFlags"] = flags;

    if (storedScale != 0)
        result["scale"] = storedScale;

    if (colorBySubcategory)
        result["colorBySubCat"] = 1;
    else
        result["color"] = lineColor.GetValue();

    result["fillColor"] = fillColor.GetValue();

#if defined(NEEDSWORKLINESTYLE_FILLCOLOR)
    // If we don't have a solid color fill, set fill to match line color...
    if (FillDisplay::Never == params.GetFillDisplay() || nullptr != params.GetGradient())
        {
        if (params.IsLineColorFromSubCategoryAppearance())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: v10Symbol->m_fillBySubCategory = true;
        else
            v10Symbol->m_fillColor = params.GetLineColor().GetValue();
        }
    else
        {
        if (params.IsFillColorFromSubCategoryAppearance())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: v10Symbol->m_fillBySubCategory = true;
        else if (params.IsFillColorFromViewBackground())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: Do we need to support bg color fill for symbols?
        else
            v10Symbol->m_fillColor = params.GetFillColor().GetValue();
        }
#endif

    result["weight"] = weight;

#if defined(NEEDSWORKLINESTYLE_FILLCOLOR)
    if (params.IsWeightFromSubCategoryAppearance())
        v10Symbol->m_weight = 0; // NEEDSWORK: v10Symbol->m_weightBySubCategory = true;
    else
        v10Symbol->m_weight = params.GetWeight();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsSymbolComponent::SaveToJson(Json::Value& result) const
    {
    LsComponent::SaveToJson(result);

#if defined(NEEDSWORKLINESTYLE_FILLCOLOR)
    // If we don't have a solid color fill, set fill to match line color...
    if (FillDisplay::Never == params.GetFillDisplay() || nullptr != params.GetGradient())
        {
        if (params.IsLineColorFromSubCategoryAppearance())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: v10Symbol->m_fillBySubCategory = true;
        else
            v10Symbol->m_fillColor = params.GetLineColor().GetValue();
        }
    else
        {
        if (params.IsFillColorFromSubCategoryAppearance())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: v10Symbol->m_fillBySubCategory = true;
        else if (params.IsFillColorFromViewBackground())
            v10Symbol->m_fillColor = 0; // NEEDSWORK: Do we need to support bg color fill for symbols?
        else
            v10Symbol->m_fillColor = params.GetFillColor().GetValue();
        }
#endif

#if defined(NEEDSWORKLINESTYLE_FILLCOLOR)
    if (params.IsWeightFromSubCategoryAppearance())
        v10Symbol->m_weight = 0; // NEEDSWORK: v10Symbol->m_weightBySubCategory = true;
    else
        v10Symbol->m_weight = params.GetWeight();
#endif

    SaveSymbolDataToJson(result, m_symBase, m_symSize, m_geomPartId, m_symFlags, m_storedScale, m_lineColorByLevel, m_lineColor, m_fillColor, false, m_weight);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj)
    {
    InitializeJsonObject (jsonObj, m_location.GetComponentId(), m_attributes, (uint32_t) m_unitDef); // WIP: m_unitDef double --> UInt32 conversion
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj, LsComponentId componentId, uint32_t flags, double unitDefinition)
    {
    jsonObj.clear();

    jsonObj[DGNPROPERTYBLOB_CompId] = componentId.GetValue();
    jsonObj[DGNPROPERTYBLOB_CompType] = static_cast <uint16_t> (componentId.GetType());
    jsonObj[DGNPROPERTYBLOB_Flags] = flags;
    jsonObj[DGNPROPERTYBLOB_UnitDef] = unitDefinition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
double LsDefinition::GetUnitDef (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_UnitDef].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
uint32_t LsDefinition::GetAttributes (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_Flags].asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponentId LsDefinition::GetComponentId (Json::Value& lsDefinition)
    {
    LsComponentType typeValue = (LsComponentType)lsDefinition[DGNPROPERTYBLOB_CompType].asUInt();
    if (!LsComponent::IsValidComponentType(typeValue))
        {
        BeAssert(LsComponent::IsValidComponentType(typeValue));
        typeValue = LsComponentType::Unknown;
        }

    uint32_t idValue = lsDefinition[DGNPROPERTYBLOB_CompId].asUInt();
    return LsComponentId(typeValue, idValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentId LsComponent::Import(LsComponentId sourceId, DgnImportContext& importer)
    {
    if (!importer.IsBetweenDbs())
        return sourceId;

    LsComponentId result = importer.FindLineStyleComponentId(sourceId);
    if (result.IsValid())
        return result;

    LsComponentPtr srcComponent = importer.GetSourceDb().Styles().LineStyles().GetLsComponent(sourceId);
    if (!srcComponent.IsValid())
        return LsComponentId();

    LsComponentPtr clonedComponent = srcComponent->_Import(importer);
    LsComponentId  clonedId = clonedComponent->GetId();
    importer.AddLineStyleComponentId(sourceId, clonedId);
    return clonedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsComponent::GetImportedComponent(LsComponentId sourceId, DgnImportContext& importer)
    {
    LsComponentId   resultId = Import(sourceId, importer);
    if (!resultId.IsValid())
        return nullptr;

    return importer.GetDestinationDb().Styles().LineStyles().GetLsComponent(resultId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        Utf8String descr = stmt.GetValueText(params.GetSelectIndex(PROPNAME_Descr));
        Utf8String data = stmt.GetValueText(params.GetSelectIndex(PROPNAME_Data));

        //  m_data.Init(baseModelId, source, descr);
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LineStyleElement::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<LineStyleElementCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    m_data = other->m_data;
    m_description = other->m_description;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    BeAssert(0 < m_data.size());
    if (m_data.size() <= 0)
        return DgnDbStatus::BadArg;

    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Data), m_data.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;

    if (m_description.SizeInBytes() > 0 && ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Descr), m_description.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_BindInsertParams(BeSQLite::EC::ECSqlStatement&stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_BindUpdateParams(BeSQLite::EC::ECSqlStatement&stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
size_t LineStyleElement::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr select = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_LineStyle));
    if (!select.IsValid())
        return 0;
    
    if (BE_SQLITE_ROW != select->Step())
        return 0;

    return static_cast<size_t>(select->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleElement::Iterator LineStyleElement::MakeIterator(DgnDbR db)
    {
    Iterator iter;
    iter.Prepare(db, "SELECT ECInstanceId, Code.[Value], Descr, Data FROM " DGN_SCHEMA(DGN_CLASSNAME_LineStyle), 0);

    return iter;
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(LineStyleHandler);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnStyleId DgnImportContext::RemapLineStyleId(DgnStyleId sourceId)
    {
    if (!IsBetweenDbs())
        return sourceId;

    DgnStyleId dest = FindLineStyleId(sourceId);
    if (dest.IsValid())
        return dest;

    
#if defined(NEEDSWORK_LINESTYLES) //  importers are not tested so don't risk passing along bad data.
    dest = LineStyleElement::ImportLineStyle(sourceId, *this);
    AddLineStyleId(sourceId, dest);
#endif

    return dest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentId DgnImportContext::FindLineStyleComponentId(LsComponentId sourceId) const
    {
    auto const& iter = m_importedComponents.find(sourceId);
    if (iter == m_importedComponents.end())
        return LsComponentId();

    return LsComponentId(sourceId.GetType(), iter->second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void DgnImportContext::AddLineStyleComponentId(LsComponentId sourceId, LsComponentId targetId)
    {
    BeAssert(sourceId.GetType() == targetId.GetType());
    BeAssert(m_importedComponents.find(sourceId) == m_importedComponents.end());
    m_importedComponents[sourceId] = targetId.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnStyleId LineStyleElement::ImportLineStyle(DgnStyleId srcStyleId, DgnImportContext& importer)
    {
    //  See if we already have a line style with the same code in the destination Db.
    //  If so, we'll map the source line style to it.  
    LineStyleElementCPtr srcStyle = LineStyleElement::Get(importer.GetSourceDb(), srcStyleId);
    BeAssert(srcStyle.IsValid());
    if (!srcStyle.IsValid())
        {
        BeAssert(false && "invalid source line style ID");
        return DgnStyleId();
        }

    DgnStyleId dstStyleId = QueryId(importer.GetDestinationDb(), srcStyle->GetName().c_str());
    if (dstStyleId.IsValid())
        {
        //  *** TBD: Check if the line style definitions match. If not, rename and remap
        importer.AddLineStyleId(srcStyleId, dstStyleId);
        return dstStyleId;
        }

    Utf8String name(srcStyle->GetName());
    Utf8String  data (srcStyle->GetData());

    Json::Value  jsonObj (Json::objectValue);
    if (!Json::Reader::Parse(data, jsonObj))
        return DgnStyleId();

    LsComponentId compId = LsDefinition::GetComponentId (jsonObj);
    compId = LsComponent::Import(compId, importer);
    if (!compId.IsValid())
        {
        //  *** TBD: Check if the line style definitions match. If not, rename and remap
        BeAssert(false && "unable to import component for line style");
        return DgnStyleId();
        }

    BentleyStatus result = importer.GetDestinationDb().Styles().LineStyles().Insert(dstStyleId, name.c_str(), compId, LsDefinition::GetAttributes(jsonObj), LsDefinition::GetAttributes(jsonObj));
    return (result == BSISUCCESS) ? dstStyleId : DgnStyleId();
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

