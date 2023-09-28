/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

Utf8CP LsJsonHelpers::CompId                = "compId";

#define PROPNAME_Description "Description"
#define PROPNAME_Data "Data"

static Utf8CP DGNPROPERTYBLOB_CompId                = "compId";
static Utf8CP DGNPROPERTYBLOB_CompType              = "compType";
static Utf8CP DGNPROPERTYBLOB_Flags                 = "flags";
static Utf8CP DGNPROPERTYBLOB_UnitDef               = "unitDef";

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double LsJsonHelpers::GetDouble(JsonValueCR json, CharCP fieldName, double defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint32_t LsJsonHelpers::GetUInt32(JsonValueCR json, CharCP fieldName, uint32_t defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int32_t LsJsonHelpers::GetInt32(JsonValueCR json, CharCP fieldName, int32_t defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint64_t LsJsonHelpers::GetUInt64(JsonValueCR json, CharCP fieldName, uint64_t defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asUInt64();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String LsJsonHelpers::GetString(JsonValueCR json, CharCP fieldName, CharCP defaultValue)
    {
    Json::Value def(defaultValue);
    return json.get(fieldName, def).asString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
LsComponentId LsJsonHelpers::GetComponentId(JsonValueCR json, CharCP typeName, CharCP idName, LsComponentType defaultType)
    {
    uint32_t idValue = GetUInt32(json, idName, 0);
    int32_t typeValue = GetInt32(json, typeName, (int32_t)defaultType);
    return LsComponentId(LsComponentType(typeValue), idValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void V10ComponentBase::GetDescription (Utf8P target) const
    {
    BeStringUtilities::Strncpy(target, LS_MAX_DESCR, m_descr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void V10ComponentBase::SetVersion()
    {
    m_version = InitialDgnDb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void V10ComponentBase::SetDescription (Utf8CP source)
    {
    memset (m_descr, 0, sizeof (m_descr));
    BeStringUtilities::Strncpy(m_descr, LS_MAX_DESCR, source);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void LsPointComponent::SaveLineCodeIdToJson(JsonValueR json, LsComponentId patternId)
    {
    if (patternId.GetType() != LsComponentType::LineCode)
        json["lcType"] = (int)patternId.GetType();
    json["lcId"] = patternId.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void LsPointComponent::SaveSymbolIdToJson(JsonValueR json, LsComponentId symbolId)
    {
    if (symbolId.GetType() != LsComponentType::PointSymbol)
        json["symType"] = (int)symbolId.GetType();

    json["symId"] = symbolId.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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

    *newComp = pSym;
    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void LsSymbolComponent::SaveSymbolDataToJson(Json::Value& result, DPoint3dCR base, DPoint3dCR size, DgnGeometryPartId const& geomPartId, int32_t flags, double storedScale) 
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

    result["geomPartId"] = geomPartId.ToHexStr();
    result["symFlags"] = flags;

    if (storedScale != 0)
        result["scale"] = storedScale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void LsSymbolComponent::SaveToJson(Json::Value& result) const
    {
    LsComponent::SaveToJson(result);

    SaveSymbolDataToJson(result, m_symBase, m_symSize, m_geomPartId, m_symFlags, m_storedScale);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj)
    {
    InitializeJsonObject (jsonObj, m_location.GetComponentId(), m_attributes, (uint32_t) m_unitDef); // WIP: m_unitDef double --> UInt32 conversion
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//--------------+------------------------------------------------------------------------
double LsDefinition::GetUnitDef (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_UnitDef].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+------------------------------------------------------------------------
uint32_t LsDefinition::GetAttributes (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_Flags].asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
LsComponentId LsComponent::Import(LsComponentId sourceId, DgnImportContext& importer)
    {
    if (!importer.IsBetweenDbs())
        return sourceId;

    LsComponentId result = importer.FindLineStyleComponentId(sourceId);
    if (result.IsValid())
        return result;

    LsComponentPtr srcComponent = importer.GetSourceDb().LineStyles().GetLsComponent(sourceId);
    if (!srcComponent.IsValid())
        return LsComponentId();

    LsComponentPtr clonedComponent = srcComponent->_Import(importer);
    LsComponentId  clonedId = clonedComponent->GetId();
    importer.AddLineStyleComponentId(sourceId, clonedId);
    return clonedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
LsComponentPtr LsComponent::GetImportedComponent(LsComponentId sourceId, DgnImportContext& importer)
    {
    LsComponentId   resultId = Import(sourceId, importer);
    if (!resultId.IsValid())
        return nullptr;

    return importer.GetDestinationDb().LineStyles().GetLsComponent(resultId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t LineStyleElement::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr select = db.GetPreparedECSqlStatement("SELECT count(*) FROM " BIS_SCHEMA(BIS_CLASS_LineStyle));
    if (!select.IsValid())
        return 0;
    
    if (BE_SQLITE_ROW != select->Step())
        return 0;

    return static_cast<size_t>(select->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
LineStyleElement::Iterator LineStyleElement::MakeIterator(DgnDbR db)
    {
    Iterator iter;
    iter.Prepare(db, "SELECT ECInstanceId,[CodeValue],Description,Data FROM " BIS_SCHEMA(BIS_CLASS_LineStyle), 0);

    return iter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_OnInsert()
    {
    auto status = T_Super::_OnInsert();
    if (DgnDbStatus::Success != status)
        return status;

    Json::Value dataObj(Json::objectValue);
    if (!Json::Reader::Parse(GetData(), dataObj))
        return DgnDbStatus::Success;

    DgnDbR db = GetDgnDb();
    LsDefinition* lsDef = new LsDefinition(GetName().c_str(), db, dataObj, DgnStyleId(GetElementId().GetValue()));
    db.LineStyles().GetCache().AddIdEntry(lsDef);

    return DgnDbStatus::Success;
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(LineStyleHandler);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnStyleId DgnImportContext::_RemapLineStyleId(DgnStyleId sourceId)
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
// @bsimethod
//---------------------------------------------------------------------------------------
LsComponentId DgnImportContext::FindLineStyleComponentId(LsComponentId sourceId) const
    {
    auto const& iter = m_importedComponents.find(sourceId);
    if (iter == m_importedComponents.end())
        return LsComponentId();

    return LsComponentId(sourceId.GetType(), iter->second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnImportContext::AddLineStyleComponentId(LsComponentId sourceId, LsComponentId targetId)
    {
    BeAssert(sourceId.GetType() == targetId.GetType());
    BeAssert(m_importedComponents.find(sourceId) == m_importedComponents.end());
    m_importedComponents[sourceId] = targetId.GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    BentleyStatus result = importer.GetDestinationDb().LineStyles().Insert(dstStyleId, name.c_str(), compId, LsDefinition::GetAttributes(jsonObj), LsDefinition::GetAttributes(jsonObj));
    return (result == BSISUCCESS) ? dstStyleId : DgnStyleId();
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

