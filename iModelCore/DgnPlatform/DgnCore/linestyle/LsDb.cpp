/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

static Utf8CP DGNPROPERTYBLOB_CompId                = "compId";
static Utf8CP DGNPROPERTYBLOB_CompType              = "compType";
static Utf8CP DGNPROPERTYBLOB_Flags                 = "flags";
static Utf8CP DGNPROPERTYBLOB_UnitDef               = "unitDef";


//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void V10ComponentBase::GetDescription (Utf8P target) const
    {
    BeStringUtilities::Strncpy(target, LS_MAX_DESCR, m_descr);
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
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsStrokePatternComponent::CreateRscFromDgnDb(LineCodeRsc** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::LineCode(), componentId.GetValue(), 0))
        return BSIERROR;

    V10LineCode* lineCodeData = (V10LineCode*)_alloca (propertySize);

    project.QueryProperty(lineCodeData, propertySize, LineStyleProperty::LineCode(), componentId.GetValue(), 0);
    BeAssert (propertySize == V10LineCode::GetBufferSize(lineCodeData->m_nStrokes));

    size_t  rscSize = LC_RSCSIZE(lineCodeData->m_nStrokes);
    LineCodeRsc*    rsc = (LineCodeRsc*)bentleyAllocator_malloc(rscSize);
    memset(rsc,0,rscSize);

    lineCodeData->GetDescription (rsc->descr);

    rsc->phase = lineCodeData->m_phase;
    rsc->options = lineCodeData->m_options;
    rsc->maxIterate = lineCodeData->m_maxIterate;

    rsc->nStrokes = lineCodeData->m_nStrokes;

    for (uint32_t index = 0; index < lineCodeData->m_nStrokes; ++index)
        {
        rsc->stroke[index].length       = lineCodeData->m_stroke[index].m_length;
        rsc->stroke[index].width        = lineCodeData->m_stroke[index].m_width;
        rsc->stroke[index].endWidth     = lineCodeData->m_stroke[index].m_endWidth;
        rsc->stroke[index].strokeMode   = lineCodeData->m_stroke[index].m_strokeMode;
        rsc->stroke[index].widthMode    = lineCodeData->m_stroke[index].m_widthMode;
        rsc->stroke[index].capMode      = lineCodeData->m_stroke[index].m_capMode;
        }

    *rscOut = rsc;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
static uint32_t  convertComponentTypeToRscType(bool useRscTypes, uint32_t componentType)
    {
    return componentType;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsPointComponent::CreateRscFromDgnDb(LinePointRsc** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::LinePoint(), componentId.GetValue(), 0))
        return BSIERROR;

    V10LinePoint* linePointData = (V10LinePoint*)_alloca (propertySize);

    project.QueryProperty(linePointData, propertySize, LineStyleProperty::LinePoint(), componentId.GetValue(), 0);
    BeAssert (propertySize == V10LinePoint::GetBufferSize(linePointData->m_nSymbols));

    size_t  rscSize = LP_RSCSIZE(linePointData->m_nSymbols);
    LinePointRsc*    rsc = (LinePointRsc*)bentleyAllocator_malloc(rscSize);
    memset(rsc,0,rscSize);

    linePointData->GetDescription (rsc->descr);

    uint32_t numberSymbols = linePointData->m_nSymbols;

    rsc->nSym = numberSymbols;

    rsc->lcType = convertComponentTypeToRscType(useRscComponentTypes, linePointData->m_lcType);
    rsc->lcID = linePointData->m_lcID;

    for (uint32_t index = 0; index < numberSymbols; ++index)
        {
        rsc->symbol[index].symType  = convertComponentTypeToRscType(useRscComponentTypes, linePointData->m_symbol[index].m_symType);
        rsc->symbol[index].symID    = linePointData->m_symbol[index].m_symID;
        rsc->symbol[index].strokeNo = linePointData->m_symbol[index].m_strokeNo;
        rsc->symbol[index].mod1     = linePointData->m_symbol[index].m_mod1;
        rsc->symbol[index].xOffset  = linePointData->m_symbol[index].m_xOffset;
        rsc->symbol[index].yOffset  = linePointData->m_symbol[index].m_yOffset;
        rsc->symbol[index].zAngle   = linePointData->m_symbol[index].m_zAngle;
        }

    *rscOut = rsc;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsCompoundComponent::CreateRscFromDgnDb(LineStyleRsc** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::Compound(), componentId.GetValue(), 0))
        return BSIERROR;

    V10Compound* compoundData = (V10Compound*)_alloca (propertySize);

    project.QueryProperty(compoundData, propertySize, LineStyleProperty::Compound(), componentId.GetValue(), 0);

    uint32_t numberComponents = compoundData->m_nComponents;
    size_t  rscSize = LS_RSCSIZE(numberComponents);

    LineStyleRsc*    rsc = (LineStyleRsc*)bentleyAllocator_malloc(rscSize);
    memset(rsc,0,rscSize);

    BeAssert (propertySize == V10Compound::GetBufferSize(numberComponents));

    compoundData->GetDescription (rsc->descr);

    rsc->nComp = numberComponents;
    for (uint32_t index = 0; index < numberComponents; ++index)
        {
        rsc->component[index].type      = convertComponentTypeToRscType(useRscComponentTypes, compoundData->m_component[index].m_type);
        rsc->component[index].id        = compoundData->m_component[index].m_id;
        rsc->component[index].offset    = compoundData->m_component[index].m_offset;
        }

    *rscOut = rsc;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsSymbolComponent::CreateRscFromDgnDb(PointSymRsc** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
    {

    BeAssert(false && "Implement CreateRscFromDgnDb");
#if defined(NOTNOW)
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::PointSym(), componentId, 0))
        return BSIERROR;

    V10Symbol* symbolData = (V10Symbol*)_alloca (propertySize);

    project.QueryProperty(symbolData, propertySize, LineStyleProperty::PointSym(), componentId, 0);

    uint32_t graphicsId = symbolData->m_symbolId;
    uint32_t graphicsIdSize = 0;

    PointSymRsc::SymbolType    symbolType = PointSymRsc::ST_XGraphics;
    if (BE_SQLITE_ROW != project.QueryPropertySize(graphicsIdSize, LineStyleProperty::SymbolXGraphics(), graphicsId, 0))
        {
        symbolType = PointSymRsc::ST_Elements;
        project.QueryPropertySize(graphicsIdSize, LineStyleProperty::SymbolElements(), graphicsId, 0);
        }

    size_t  rscSize = sizeof (PointSymRsc) + graphicsIdSize;
    PointSymRsc* psr = (PointSymRsc*)bentleyAllocator_malloc(rscSize);
    memset(psr,0,rscSize);

    symbolData->GetDescription (psr->header.descr);

    project.QueryProperty(psr->symBuf, graphicsIdSize, 
                            PointSymRsc::ST_XGraphics == symbolType ? LineStyleProperty::SymbolXGraphics() : LineStyleProperty::SymbolElements(), graphicsId, 0);

    psr->nBytes = graphicsIdSize;

    psr->header.range = symbolData->m_range;
    psr->header.scale = symbolData->m_scale;
    psr->symFlags = symbolData->m_symFlags;
    psr->SetSymbolType (symbolType);

    *rscOut = psr;
    return BSIERROR;
#else
    return BSISUCCESS;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LsPointComponent::AddToProject (LsComponentId& newId, DgnDbR project, LinePointRsc const& lpr)
    {
    uint32_t bufferSize = V10LinePoint::GetBufferSize(lpr.nSym);
    V10LinePoint*v10LinePoint = (V10LinePoint*)_alloca(bufferSize);
    memset (v10LinePoint, 0, bufferSize);

    v10LinePoint->SetDescription(lpr.descr);
    v10LinePoint->m_nSymbols = lpr.nSym;

    v10LinePoint->m_lcType = lpr.lcType;
    v10LinePoint->m_lcID = lpr.lcID;


    for (unsigned i = 0; i < lpr.nSym; ++i)
        {
        PointSymInfo const&         sym = lpr.symbol[i];
        V10PointSymbolInfo &        v10sym = v10LinePoint->m_symbol[i];

        v10sym.m_symType = sym.symType;
        v10sym.m_symID = sym.symID;

        v10sym.m_strokeNo = sym.strokeNo;
        v10sym.m_mod1 = sym.mod1;
        v10sym.m_xOffset = sym.xOffset;
        v10sym.m_yOffset = sym.yOffset;
        v10sym.m_zAngle = sym.zAngle;
        }

    return LsComponent::AddComponentAsProperty (newId, project, LineStyleProperty::LinePoint(), v10LinePoint, bufferSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LsStrokePatternComponent::AddToProject (LsComponentId& newId, DgnDbR project, LineCodeRsc const& lcr)
    {
    uint32_t bufferSize = V10LineCode::GetBufferSize(lcr.nStrokes);
    V10LineCode*v10LineCode = (V10LineCode*)_alloca(bufferSize);
    memset (v10LineCode, 0, bufferSize);

    v10LineCode->SetDescription(lcr.descr);
    v10LineCode->m_phase = lcr.phase;
    v10LineCode->m_maxIterate = lcr.maxIterate;
    v10LineCode->m_options = lcr.options;
    v10LineCode->m_nStrokes = lcr.nStrokes;

    for (unsigned i = 0; i < lcr.nStrokes; ++i)
        {
        StrokeData const& stroke = lcr.stroke[i];
        V10StrokeData & v10stroke = v10LineCode->m_stroke[i];

        v10stroke.m_length = stroke.length;
        v10stroke.m_width = stroke.width;
        v10stroke.m_endWidth = stroke.endWidth;

        v10stroke.m_strokeMode = stroke.strokeMode;
        v10stroke.m_widthMode = stroke.widthMode;
        v10stroke.m_capMode = stroke.capMode;
        v10stroke.m_bReserved = 0;
        }

    return LsComponent::AddComponentAsProperty(newId, project, LineStyleProperty::LineCode(), v10LineCode, bufferSize);
    }

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleUpgradeProcessor::LineStyleUpgradeProcessor (DgnDbR project, DgnElementDescrCP components) :
                            m_v8Components(components), m_dgnDb (project)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradeLineStyleV8toV10 (LStyleNameEntryElm const& lStyle, Utf8CP alternateName)
    {
    BeAssert (lStyle.flags.isElement);

    uint32_t v10ComponentId;
    UpgradeLsComponent(v10ComponentId, lStyle.refersTo, remapRscTypeToElmType(lStyle.type));

    Utf8String  lineStyleName;
    BeStringUtilities::Utf16ToUtf8 (lineStyleName, lStyle.name);

    if (NULL == alternateName)
        alternateName = lineStyleName.c_str();

    DgnStyleId  origId ((uint32_t)lStyle.id);
    DbResult result = GetDgnDb().Styles ().LineStyles().InsertWithId (origId, alternateName, v10ComponentId, lStyle.type, 
                        lStyle.styleFlags, lStyle.auxInfo[1]/LSUNIT_FACTOR);

    if (BE_SQLITE_DONE == result)
        return LINESTYLE_STATUS_Success;

    return IsConstraintDbResult(result) ? LINESTYLE_STATUS_SQLITE_Constraint : LINESTYLE_STATUS_SQLITE_Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradeLsComponent (uint32_t& v10Id, uint64_t componentId, LsComponentType type)
    {
    if (LsComponentType::Internal == type)
        {
        v10Id = (uint32_t)componentId;
        return LINESTYLE_STATUS_Success;
        }

    bmap <uint64_t, uint32_t>::iterator it = m_idMap.find(componentId);
    if (it != m_idMap.end())
        {
        v10Id = it->second;
        return LINESTYLE_STATUS_Success;
        }

    DgnElementDescrCP v8component = FindComponent(componentId);
    if (NULL == v8component)
        return LINESTYLE_STATUS_ComponentNotFound;

    LineStyleStatus retval = LINESTYLE_STATUS_Success;
    LStyleDefEntryElm const& el= (LStyleDefEntryElm const&)v8component->Element();
    switch ((LsComponentType)el.type)
        {
        case LsComponentType::LineCode:
            retval = UpgradeLineCode (v10Id, el);
            break;

        case LsComponentType::Compound:
            retval = UpgradeCompoundType (v10Id, el);
            break;

        case LsComponentType::LinePoint:
            retval = UpgradeLinePoint (v10Id, el);
            break;

        case LsComponentType::PointSymbol:
            retval = UpgradePointSymbol (v10Id, el);
            break;
        }

    m_idMap[componentId] = v10Id;
    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
RefCountedPtr<LineStyleUpgradeProcessor> LineStyleUpgradeProcessor::Create (DgnDbR project, DgnElementDescrCP components)
    {
    return new LineStyleUpgradeProcessor (project, components);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj)
    {
    InitializeJsonObject (jsonObj, m_location.GetComponentId(), m_location.GetComponentType(),
                          m_attributes, (uint32_t) m_unitDef); // WIP: m_unitDef double --> UInt32 conversion
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj, LsComponentId componentId, LsComponentType componentType, uint32_t flags, double unitDefinition)
    {
    jsonObj.clear();

    jsonObj[DGNPROPERTYBLOB_CompId] = componentId.GetValue();
    jsonObj[DGNPROPERTYBLOB_CompType] = static_cast <uint16_t> (componentType);  //  defined(NOTNOW) (uint32_t)remapRscTypeToElmType(componentType);
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
LsComponentType LsDefinition::GetComponentType (Json::Value& lsDefinition)
    {
    LsComponentType retval = (LsComponentType)lsDefinition[DGNPROPERTYBLOB_CompType].asUInt();
    if (!LsComponent::IsValidComponentType(retval))
        {
        BeAssert(LsComponent::IsValidComponentType(retval));
        retval = LsComponentType::Unknown;
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponentId LsDefinition::GetComponentId (Json::Value& lsDefinition)
    {
    return LsComponentId(lsDefinition[DGNPROPERTYBLOB_CompId].asUInt());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void LsComponent::QueryComponentIds(bset<LsComponentTypeAndId>& ids, DgnDbCR project, LsComponentType lsType)
    {
    //  No default constructor so we have to initialize it.
    LineStyleProperty::ComponentProperty spec = LineStyleProperty::Compound();

    switch(lsType)
        {
        case LsComponentType::Compound:
            break;
        case LsComponentType::LineCode:
            spec = LineStyleProperty::LineCode();
            break;
        case LsComponentType::LinePoint:
            spec = LineStyleProperty::LinePoint();
            break;
        case LsComponentType::PointSymbol:
            spec = LineStyleProperty::PointSym();
            break;

        default:
            BeAssert(false && "bad lsType argument to QueryComponentIds");
            return;
        }

    Statement stmt;
    stmt.Prepare (project, SqlPrintfString("SELECT Id FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=?"));

    stmt.BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(2, spec.GetName(), Statement::MakeCopy::No);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        LsComponentTypeAndId     componentId;
        componentId.m_id = stmt.GetValueInt(0);
        componentId.m_type = (uint32_t)lsType;
        ids.insert(componentId);
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsStrokePatternComponent::GetRscFromDgnDb(LineCodeRscPtr& ptr, DgnDbR project, LsComponentId componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsCompoundComponent::GetRscFromDgnDb(LineStyleRscPtr& ptr, DgnDbR project, LsComponentId componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsSymbolComponent::GetRscFromDgnDb(PointSymRscPtr& ptr, DgnDbR project, LsComponentId componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsPointComponent::GetRscFromDgnDb(LinePointRscPtr& ptr, DgnDbR project, LsComponentId componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
LineCodeRscPtr::~LineCodeRscPtr() { bentleyAllocator_free(m_data); }
LinePointRscPtr::~LinePointRscPtr() { bentleyAllocator_free(m_data); }
LineStyleRscPtr::~LineStyleRscPtr() { bentleyAllocator_free(m_data); }
PointSymRscPtr::~PointSymRscPtr() { bentleyAllocator_free(m_data); }

